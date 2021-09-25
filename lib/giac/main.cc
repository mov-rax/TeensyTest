// Interface: *GUI*, main.cpp, console.*, *Provider*
#include <stdio.h>
#include <fxcg/keyboard.h>
#include "giacPCH.h"
#include <fxcg/display.h>
#include <fxcg/file.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include <fxcg/misc.h>
#include <fxcg/app.h>
#include <fxcg/serial.h>
extern "C" {
#include <fxcg/rtc.h>
}
#include <fxcg/heap.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alloca.h>
#include <ctype.h>

#include "console.h"
  //#include "memmgr.h"
#include "catalogGUI.hpp"
#include "fileGUI.hpp"
#include "textGUI.hpp"
#include "graphicsProvider.hpp"
#include "constantsProvider.hpp"
#include "kdisplay.h"
#include "input_lexer.h"
#include "input_parser.h"
#include "main.h"
#define GIAC_HISTORY_SIZE 8
#define GIAC_HISTORY_MAX_TAILLE 32

using namespace giac;

void do_restart(){
  giac::_restart(gen(vecteur(0),_SEQ__VECT),contextptr);
}

void chk_restart(){
  drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
  if (confirm(chk_restart_string1,chk_restart_string2)==KEY_CTRL_F6)
    do_restart();
}


int exproffset = 0;
int esc_flag=0;
int run_startup_script_again=0;
//extern void set_rnd_seed(int);
int custom_key_to_handle;
int custom_key_to_handle_modifier;
int has_drawn_graph = 0;


extern char* outputRedirectBuffer;
extern int remainingBytesInRedirect;

//const char * keywords[]={"do","faire","for","if","return","while"}; // added to lexer_tab_int.h

int find_color(const char * s){
  if (s[0]=='"')
    return 4;
  if (!isalpha(s[0]))
    return 0;
  char buf[256];
  const char * ptr=s;
  for (int i=0;i<255 && (isalphanum(*ptr) || *ptr=='_'); ++i){
    ++ptr;
  }
  strncpy(buf,s,ptr-s);
  buf[ptr-s]=0;
  if (strcmp(buf,"def")==0)
    return 1;
  //int pos=dichotomic_search(keywords,sizeof(keywords),buf);
  //if (pos>=0) return 1;
  gen g;
  int token=find_or_make_symbol(buf,g,0,false,contextptr);
  //*logptr(contextptr) << s << " " << buf << " " << token << " " << g << endl;
  if (token==T_UNARY_OP || token==T_UNARY_OP_38 || token==T_LOGO)
    return 3;
  if (token==T_NUMBER)
    return 2;
  if (token!=T_SYMBOL)
    return 1;
  return 0;
}

#if 0
const char * unary_function_ptr_name(void * ptr){
  const giac::unary_function_ptr * u=(const unary_function_ptr *) ptr;
  return u->ptr()->s;
}
#endif

void stop(const char * s){
}

int main(){
  context ct;
  contextptr=&ct;
  SetQuitHandler(save_session); // automatically save session when exiting
  turtle();
#ifdef TURTLETAB
  turtle_stack_size=0;
#else
  turtle_stack(); // required to init turtle
#endif
  int key;
  Console_Init();
  Bdisp_AllClr_VRAM();
  Bdisp_EnableColor(1);
  EnableStatusArea(0);
  DefineStatusAreaFlags(3, SAF_BATTERY | SAF_TEXT | SAF_GLYPH | SAF_ALPHA_SHIFT, 0, 0);
  rand_seed(RTC_GetTicks(),contextptr);
  VRAM_base = (color_t*)GetVRAMAddress();
  restore_session("session");
  //GetKey(&key);
  Console_Disp();
  // GetKey(&key);
  // disable Catalog function throughout the add-in, as we don't know how to make use of it:
  Bkey_SetAllFlags(0x80);
  unsigned char *expr=0;
  while(1){
    if((expr=Console_GetLine())==NULL) stop("memory error");
    if (strcmp((const char *)expr,"restart")==0){
      if (confirm(main_string1,main_string2)!=KEY_CTRL_F6){
	Console_Output((const unsigned char *)" cancelled");
	Console_NewLine(LINE_TYPE_OUTPUT,1);
	//GetKey(&key);
	Console_Disp();
	continue;
      }
    }
    // should save in another file
    if (strcmp((const char *)expr,"=>")==0 || strcmp((const char *)expr,"=>\n")==0){
      save_session();
      Console_Output((unsigned char*)"Session saved");
    }
    else 
      run((char *)expr);
    //print_mem_info();
    Console_NewLine(LINE_TYPE_OUTPUT,1);
    //GetKey(&key);
    Console_Disp();
  }
  for(;;)
    GetKey(&key);
  return 1;
}


bool islogo(const gen & g){
  if (g.type!=_VECT || g._VECTptr->empty()) return false;
  if (g.subtype==_LOGO__VECT) return true;
  const vecteur & v=*g._VECTptr;
  if (islogo(v.back()))
    return true;
  for (size_t i=0;i<v.size();++i){
    if (v[i].type==_VECT && v[i].subtype==_LOGO__VECT)
      return true;
  }
  return false;
}

bool ispnt(const gen & g){
  if (g.is_symb_of_sommet(giac::at_pnt))
    return true;
  if (g.type!=_VECT || g._VECTptr->empty())
    return false;
  return ispnt(g._VECTptr->back());
}

giac::gen eqw(const giac::gen & ge,bool editable){
  bool edited=false;
#ifdef CURSOR
  Cursor_SetFlashOff();
#endif
  giac::gen geq(_copy(ge,contextptr));
  // if (ge.type!=giac::_DOUBLE_ && giac::has_evalf(ge,geq,1,contextptr)) geq=giac::symb_equal(ge,geq);
  int line=-1,col=-1,nlines=0,ncols=0,listormat=0;
  xcas::Equation eq(0,0,geq);
  giac::eqwdata eqdata=xcas::Equation_total_size(eq.data);
  if (eqdata.dx>1.5*LCD_WIDTH_PX || eqdata.dy>.7*LCD_HEIGHT_PX){
    if (eqdata.dx>2.25*LCD_WIDTH_PX || eqdata.dy>2*LCD_HEIGHT_PX)
      eq.attr=giac::attributs(14,COLOR_WHITE,COLOR_BLACK);
    else
      eq.attr=giac::attributs(16,COLOR_WHITE,COLOR_BLACK);
    eq.data=0; // clear memory
    eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
    eqdata=xcas::Equation_total_size(eq.data);
  }
  int dx=(eqdata.dx-LCD_WIDTH_PX)/2,dy=LCD_HEIGHT_PX-2*STATUS_AREA_PX+eqdata.y;
  if (geq.type==_VECT){
    nlines=geq._VECTptr->size();
    if (eqdata.dx>=LCD_WIDTH_PX)
      dx=-20; // line=nlines/2;
    //else
    if (geq.subtype!=_SEQ__VECT){
      line=0;
      listormat=1;
      if (ckmatrix(geq)){
	ncols=geq._VECTptr->front()._VECTptr->size();
	if (eqdata.dy>=LCD_HEIGHT_PX-STATUS_AREA_PX)
	  dy=eqdata.y+eqdata.dy+32;// col=ncols/2;
	// else
	col=0;
	listormat=2;
      }
    }
  }
  if (!listormat){
    xcas::Equation_select(eq.data,true);
    xcas::eqw_select_down(eq.data);
  }
  //cout << eq.data << endl;
  int firstrun=2;
  for (;;){
#if 1
    if (firstrun==2){
      DefineStatusMessage((char*)(lang?"EXE: quitte, resultat dans last":"EXE: quit, result stored in last"), 1, 0, 0);
      EnableStatusArea(2);
      DisplayStatusArea();
      firstrun=1;
    }
    else
      set_xcas_status();
#else
    DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
    EnableStatusArea(2);
    DisplayStatusArea();
#endif
    gen value;
    if (listormat) // select line l, col c
      xcas::eqw_select(eq.data,line,col,true,value);
    if (eqdata.dx>LCD_WIDTH_PX){
      if (dx<-20)
	dx=-20;
      if (dx>eqdata.dx-LCD_WIDTH_PX+20)
	dx=eqdata.dx-LCD_WIDTH_PX+20;
    }
#define EQW_TAILLE 18
    if (eqdata.dy>LCD_HEIGHT_PX-3*EQW_TAILLE){
      if (dy-eqdata.y<LCD_HEIGHT_PX-3*EQW_TAILLE)
	dy=eqdata.y+LCD_HEIGHT_PX-3*EQW_TAILLE;
      if (dy-eqdata.y>eqdata.dy+32)
	dy=eqdata.y+eqdata.dy+32;
    }
    drawRectangle(0, STATUS_AREA_PX, LCD_WIDTH_PX, LCD_HEIGHT_PX-STATUS_AREA_PX,COLOR_WHITE);
    // Bdisp_AllClr_VRAM();
    int save_clip_ymin=clip_ymin;
    clip_ymin=STATUS_AREA_PX;
    xcas::display(eq,dx,dy);
    string menu(" ");
    menu += menu_f1;
    while (menu.size()<6)
      menu += " ";
    menu += " | ";
    menu += string(menu_f2);
    while (menu.size()<13)
      menu += " ";
    menu += " | edit+-| cmds | A<>a | eval";
    PrintMini(0,58,menu.c_str(),4);
    //draw_menu(2);
    clip_ymin=save_clip_ymin;
    int keyflag = GetSetupSetting( (unsigned int)0x14);
    bool alph=keyflag==4||keyflag==0x84||keyflag==8||keyflag==0x88;
    if (firstrun){ // workaround for e.g. 1+x/2 partly not displayed
      firstrun=0;
      continue;
    }
    int key;
    //cout << eq.data << endl;
    GetKey(&key);
    if (key==KEY_CTRL_UNDO){
      giac::swapgen(eq.undodata,eq.data);
      if (listormat){
	xcas::do_select(eq.data,true,value);
	if (value.type==_EQW){
	  gen g=eval(value._EQWptr->g,1,contextptr);
	  if (g.type==_VECT){
	    const vecteur & v=*g._VECTptr;
	    nlines=v.size();
	    if (line >= nlines)
	      line=nlines-1;
	    if (col!=-1 &&v.front().type==_VECT){
	      ncols=v.front()._VECTptr->size();
	      if (col>=ncols)
		col=ncols-1;
	    }
	    xcas::do_select(eq.data,false,value);
	    xcas::eqw_select(eq.data,line,col,true,value);
	  }
	}
      }
      continue;
    }
    if (key==KEY_CTRL_F5){
      handle_f5();
      continue;
    }
    int redo=0;
    if (listormat){
      if (key==KEY_CHAR_COMMA || key==KEY_CTRL_DEL ){
	xcas::do_select(eq.data,true,value);
	if (value.type==_EQW){
	  gen g=eval(value._EQWptr->g,1,contextptr);
	  if (g.type==_VECT){
	    edited=true; eq.undodata=xcas::Equation_copy(eq.data);
	    vecteur v=*g._VECTptr;
	    if (key==KEY_CHAR_COMMA){
	      if (col==-1 || (line>0 && line==nlines-1)){
		v.insert(v.begin()+line+1,0*v.front());
		++line; ++nlines;
	      }
	      else {
		v=mtran(v);
		v.insert(v.begin()+col+1,0*v.front());
		v=mtran(v);
		++col; ++ncols;
	      }
	    }
	    else {
	      if (col==-1 || (nlines>=3 && line==nlines-1)){
		if (nlines>=(col==-1?2:3)){
		  v.erase(v.begin()+line,v.begin()+line+1);
		  if (line) --line;
		  --nlines;
		}
	      }
	      else {
		if (ncols>=2){
		  v=mtran(v);
		  v.erase(v.begin()+col,v.begin()+col+1);
		  v=mtran(v);
		  if (col) --col; --ncols;
		}
	      }
	    }
	    geq=gen(v,g.subtype);
	    key=0; redo=1;
	    // continue;
	  }
	}
      }
    }
    bool ins=key==KEY_CHAR_STORE  || key==KEY_CHAR_RPAR || key==KEY_CHAR_LPAR || key==KEY_CHAR_COMMA || key==KEY_CTRL_PASTE;
    int xleft,ytop,xright,ybottom,gselpos; gen * gsel=0,*gselparent=0;
    ustl::string varname;
    if (key==KEY_CTRL_CLIP){
      xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,0);
      if (gsel==0)
	gsel==&eq.data;
      // cout << "var " << g << " " << eq.data << endl;
      if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	//cout << g << ":=" << value._EQWptr->g << endl;
	copy_clipboard(value._EQWptr->g.print(contextptr),true);
	continue;
      }
    }
    if (key==KEY_CHAR_STORE){
      int keyflag = GetSetupSetting( (unsigned int)0x14);
      if (keyflag==0)
	handle_f5();
      if (inputline(lang?"Stocker la selection dans":"Save selection in",lang?"Nom de variable: ":"Variable name: ",varname,false) && !varname.empty() && isalpha(varname[0])){
	giac::gen g(varname,contextptr);
	giac::gen ge(eval(g,1,contextptr));
	if (g.type!=_IDNT){
	  invalid_varname();
	  continue;
	}
	if (ge==g || confirm_overwrite()){
	  vector<int> goto_sel;
	  xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel);
	  if (gsel==0)
	    gsel==&eq.data;
	  // cout << "var " << g << " " << eq.data << endl;
	  if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	    //cout << g << ":=" << value._EQWptr->g << endl;
	    giac::gen gg(value._EQWptr->g);
	    if (gg.is_symb_of_sommet(at_makevector))
	      gg=giac::eval(gg,1,contextptr);
	    giac::sto(gg,g,contextptr);
	  }
	}
      }
      continue;
    }
    if (key==KEY_CTRL_DEL){
      vector<int> goto_sel;
      if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	value=value._EQWptr->g;
	if (value.type==_SYMB){
	  gen tmp=value._SYMBptr->feuille;
	  if (tmp.type!=_VECT || tmp.subtype!=_SEQ__VECT){
	    xcas::replace_selection(eq,tmp,gsel,&goto_sel);
	    continue;
	  }
	}
	if (!goto_sel.empty() && gselparent && gselparent->type==_VECT && !gselparent->_VECTptr->empty()){
	  vecteur & v=*gselparent->_VECTptr;
	  if (v.back().type==_EQW){
	    gen opg=v.back()._EQWptr->g;
	    if (opg.type==_FUNC){
	      int i=0;
	      for (;i<v.size()-1;++i){
		if (&v[i]==gsel)
		  break;
	      }
	      if (i<v.size()-1){
		if (v.size()==5 && (opg==at_integrate || opg==at_sum) && i>=2)
		  v.erase(v.begin()+2,v.begin()+4);
		else
		  v.erase(v.begin()+i);
		xcas::do_select(*gselparent,true,value);
		if (value.type==_EQW){
		  value=value._EQWptr->g;
		  // cout << goto_sel << " " << value << endl; continue;
		  if (v.size()==2 && (opg==at_plus || opg==at_prod))
		    value=eval(value,1,contextptr);
		  goto_sel.erase(goto_sel.begin());
		  xcas::replace_selection(eq,value,gselparent,&goto_sel);
		  continue;
		}
	      }
	    }
	  }
	}
      }
    }
    if (key==KEY_CTRL_F3){
      if (keyflag==1){
	if (eq.attr.fontsize>=18) continue;
	xcas::do_select(eq.data,true,value);
	if (value.type==_EQW)
	  geq=value._EQWptr->g;
	if (eq.attr.fontsize==14)
	  eq.attr.fontsize=16;
	else
	  eq.attr.fontsize=18;
	redo=1;
      }
      else {
	if (alph){
	  if (eq.attr.fontsize<=14) continue;
	  xcas::do_select(eq.data,true,value);
	  if (value.type==_EQW)
	    geq=value._EQWptr->g;
	  if (eq.attr.fontsize==16)
	    eq.attr.fontsize=14;
	  else
	    eq.attr.fontsize=16;
	  redo=1;
	}
	else {
	  // Edit
	  edited=true;
	  ins=true;
	}
      }
    }
    if (key==KEY_CHAR_IMGNRY)
      key='i';
    const char keybuf[2]={(key==KEY_CHAR_PMINUS?'-':char(key)),0};
    const char * adds=(key==KEY_CHAR_PMINUS ||
		       (key==char(key) && (isalphanum(key)|| key=='.' ))
		       )?keybuf:keytostring(key,keyflag);
    translate_fkey(key);
    if ( key==KEY_CTRL_F1 || key==KEY_CTRL_F2 ||
	 (key>=KEY_CTRL_F7 && key<=KEY_CTRL_F14)){
      adds=console_menu(key,1);//alph?"simplify":(keyflag==1?"factor":"partfrac");
      if (!adds) continue;
      // workaround for infinitiy
      if (strlen(adds)>=2 && adds[0]=='o' && adds[1]=='o')
	key=KEY_CTRL_F5;      
    }
    if (key==KEY_CTRL_F5)
      adds="oo";
    if (key==KEY_CTRL_F6){
      adds=alph?"regroup":(keyflag==1?"evalf":"eval");
    }
    if (key==KEY_CHAR_MINUS)
      adds="-";
    if (key==KEY_CHAR_EQUAL)
      adds="=";
    if (key==KEY_CHAR_RECIP)
      adds="inv";
    if (key==KEY_CHAR_SQUARE)
      adds="sq";
    if (key==KEY_CHAR_POWROOT)
      adds="surd";
    if (key==KEY_CHAR_CUBEROOT)
      adds="surd";
    int addssize=adds?strlen(adds):0;
    // cout << addssize << " " << adds << endl;
    if (key==KEY_CTRL_EXE){
      if (xcas::do_select(eq.data,true,value) && value.type==_EQW){
	//cout << "ok " << value._EQWptr->g << endl;
	//DefineStatusMessage((char*)lang?"resultat stocke dans last":"result stored in last", 1, 0, 0);
	//DisplayStatusArea();
	giac::sto(value._EQWptr->g,giac::gen("last",contextptr),contextptr);
	return value._EQWptr->g;
      }
      //cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
      return geq;
    }
    if ( key!=KEY_CHAR_MINUS && key!=KEY_CHAR_EQUAL &&
	(ins || key==KEY_CHAR_PI || key==KEY_CTRL_F5 || (addssize==1 && (isalphanum(adds[0])|| adds[0]=='.' || adds[0]=='-') ) )
	){
      edited=true;
      if (line>=0 && xcas::eqw_select(eq.data,line,col,true,value)){
	ustl::string s;
	if (ins){
	  if (key==KEY_CTRL_PASTE)
	    s=paste_clipboard();
	  else {
	    if (value.type==_EQW){
	      s=value._EQWptr->g.print(contextptr);
	    }
	    else
	      s=value.print(contextptr);
	  }
	}
	else
	  s = adds;
 	ustl::string msg("Line ");
	msg += print_INT_(line+1);
	msg += " Col ";
	msg += print_INT_(col+1);
	if (inputline(msg.c_str(),0,s,false)==KEY_CTRL_EXE){
	  value=gen(s,contextptr);
	  if (col<0)
	    (*geq._VECTptr)[line]=value;
	  else
	    (*((*geq._VECTptr)[line]._VECTptr))[col]=value;
	  redo=2;
	  key=KEY_SHIFT_RIGHT;
	}
	else
	  continue;
      }
      else {
	vector<int> goto_sel;
	if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	  ustl::string s;
	  if (ins){
	    if (key==KEY_CTRL_PASTE)
	      s=paste_clipboard();
	    else {
	      s = value._EQWptr->g.print(contextptr);
	      if (key==KEY_CHAR_COMMA)
		s += ',';
	    }
	  }
	  else
	    s = adds;
	  if (inputline(value._EQWptr->g.print(contextptr).c_str(),0,s,false)==KEY_CTRL_EXE){
	    value=gen(s,contextptr);
	    //cout << value << " goto " << goto_sel << endl;
	    xcas::replace_selection(eq,value,gsel,&goto_sel);
	    firstrun=-1; // workaround, force 2 times display
	  }
	  continue;
	}
      }
    }
    if (redo){
      eq.data=0; // clear memory
      eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
      eqdata=xcas::Equation_total_size(eq.data);
      if (redo==1){
	dx=(eqdata.dx-LCD_WIDTH_PX)/2;
	dy=LCD_HEIGHT_PX-2*STATUS_AREA_PX+eqdata.y;
	if (listormat) // select line l, col c
	  xcas::eqw_select(eq.data,line,col,true,value);
	else {
	  xcas::Equation_select(eq.data,true);
	  xcas::eqw_select_down(eq.data);
	}
	continue;
      }
    }
    if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC){
      if (!edited)
	return geq;
      if (confirm(lang?"Vraiment abandonner?":"Really leave",lang?"F1: retour editeur,  F6: confirmer":"F1: back to editor,  F6: confirm")==KEY_CTRL_F6)
	return undef;
    }
    bool doit=eqdata.dx>=LCD_WIDTH_PX;
    int delta=0;
    if (listormat){
      if (key==KEY_CTRL_LEFT  || (!doit && key==KEY_SHIFT_LEFT)){
	if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	  if (col>=0){
	    --col;
	    if (col<0){
	      col=ncols-1;
	      if (line>0)
		--line;
	    }
	  }
	  else {
	    if (line>0)
	      --line;
	  }
	  xcas::eqw_select(eq.data,line,col,true,value);
	  if (doit) dx -= value._EQWptr->dx;
	}
	continue;
      }
      if (doit && key==KEY_SHIFT_LEFT){
	dx -= 20;
	continue;
      }
      if (key==KEY_CTRL_RIGHT  || (!doit && key==KEY_SHIFT_RIGHT)) {
	if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	  if (doit)
	    dx += value._EQWptr->dx;
	  if (col>=0){
	    ++col;
	    if (col==ncols){
	      col=0;
	      if (line<nlines-1)
		++line;
	    }
	  } else {
	    if (line<nlines-1)
	      ++line;
	  }
	  xcas::eqw_select(eq.data,line,col,true,value);
	}
	continue;
      }
      if (key==KEY_SHIFT_RIGHT && doit){
	dx += 20;
	continue;
      }
      doit=eqdata.dy>=LCD_HEIGHT_PX-2*STATUS_AREA_PX;
      if (key==KEY_CTRL_UP || (!doit && key==KEY_CTRL_PAGEUP)){
	if (line>0 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	  --line;
	  xcas::eqw_select(eq.data,line,col,true,value);
	  if (doit)
	    dy += value._EQWptr->dy+eq.attr.fontsize/2;
	}
	continue;
      }
      if (key==KEY_CTRL_PAGEUP && doit){
	dy += 10;
	continue;
      }
      if (key==KEY_CTRL_DOWN  || (!doit && key==KEY_CTRL_PAGEDOWN)){
	if (line<nlines-1 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	  if (doit)
	    dy -= value._EQWptr->dy+eq.attr.fontsize/2;
	  ++line;
	  xcas::eqw_select(eq.data,line,col,true,value);
	}
	continue;
      }
      if ( key==KEY_CTRL_PAGEDOWN && doit){
	dy -= 10;
	continue;
      }
    }
    else { // else listormat
      if (key==KEY_CTRL_LEFT){
	delta=xcas::eqw_select_leftright(eq,true,alph?2:0);
	// cout << "left " << delta << endl;
	if (doit) dx += (delta?delta:-20);
	continue;
      }
      if (key==KEY_SHIFT_LEFT){
	delta=xcas::eqw_select_leftright(eq,true,1);
	vector<int> goto_sel;
	if (doit) dx += (delta?delta:-20);
	continue;
      }
      if (key==KEY_CTRL_RIGHT){
	delta=xcas::eqw_select_leftright(eq,false,alph?2:0);
	// cout << "right " << delta << endl;
	if (doit)
	  dx += (delta?delta:20);
	continue;
      }
      if (key==KEY_SHIFT_RIGHT){
	delta=xcas::eqw_select_leftright(eq,false,1);
	// cout << "right " << delta << endl;
	if (doit)
	  dx += (delta?delta:20);
	// dx=eqdata.dx-LCD_WIDTH_PX+20;
	continue;
      }
      doit=eqdata.dy>=LCD_HEIGHT_PX-2*STATUS_AREA_PX;
      if (key==KEY_CTRL_UP){
	delta=xcas::eqw_select_up(eq.data);
	// cout << "up " << delta << endl;
	continue;
      }
      //cout << "up " << eq.data << endl;
      if (key==KEY_CTRL_PAGEUP && doit){
	dy=eqdata.y+eqdata.dy+20;
	continue;
      }
      if (key==KEY_CTRL_DOWN){
	delta=xcas::eqw_select_down(eq.data);
	// cout << "down " << delta << endl;
	continue;
      }
      //cout << "down " << eq.data << endl;
      if ( key==KEY_CTRL_PAGEDOWN && doit){
	dy=eqdata.y+LCD_HEIGHT_PX-STATUS_AREA_PX;
	continue;
      }
    }
    if (adds){
      edited=true;
      if (strcmp(adds,"'")==0)
	adds="diff";
      if (strcmp(adds,"^2")==0)
	adds="sq";
      if (strcmp(adds,">")==0)
	adds="simplify";
      if (strcmp(adds,"<")==0)
	adds="factor";
      if (strcmp(adds,"#")==0)
	adds="partfrac";
      string cmd(adds);
      if (cmd.size() && cmd[cmd.size()-1]=='(')
	cmd ='\''+cmd.substr(0,cmd.size()-1)+'\'';
      vector<int> goto_sel;
      if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel){
	gen op;
	int addarg=0;
	if (addssize==1){
	  switch (adds[0]){
	  case '+':
	    addarg=1;
	    op=at_plus;
	    break;
	  case '^':
	    addarg=1;
	    op=at_pow;
	    break;
	  case '=':
	    addarg=1;
	    op=at_equal;
	    break;
	  case '-':
	    addarg=1;
	    op=at_binary_minus;
	    break;
	  case '*':
	    addarg=1;
	    op=at_prod;
	    break;
	  case '/':
	    addarg=1;
	    op=at_division;
	    break;
	  case '\'':
	    addarg=1;
	    op=at_diff;
	    break;
	  }
	}
	if (op==0)
	  op=gen(cmd,contextptr);
	if (op.type==_SYMB)
	  op=op._SYMBptr->sommet;
	// cout << "keyed " << adds << " " << op << " " << op.type << endl;
	if (op.type==_FUNC){
	  edited=true;
	  // execute command on selection
	  gen tmp,value;
	  if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	    if (op==at_integrate || op==at_sum)
	      addarg=3;
	    if (op==at_limit)
	      addarg=2;
	    gen args=eval(value._EQWptr->g,1,contextptr);
	    gen vx=xthetat?t__IDNT_e:x__IDNT_e;
	    if (addarg==1)
	      args=makesequence(args,0);
	    if (addarg==2)
	      args=makesequence(args,vx_var(),0);
	    if (addarg==3)
	      args=makesequence(args,vx_var(),0,1);
	    if (op==at_surd)
	      args=makesequence(args,key==KEY_CHAR_CUBEROOT?3:4);
	    if (op==at_subst)
	      args=makesequence(args,giac::symb_equal(vx_var(),0));
	    unary_function_ptr immediate_op[]={*at_eval,*at_evalf,*at_evalc,*at_regrouper,*at_simplify,*at_normal,*at_ratnormal,*at_factor,*at_cfactor,*at_partfrac,*at_cpartfrac,*at_expand,*at_canonical_form,*at_exp2trig,*at_trig2exp,*at_sincos,*at_lin,*at_tlin,*at_tcollect,*at_texpand,*at_trigexpand,*at_trigcos,*at_trigsin,*at_trigtan,*at_halftan};
	    if (equalposcomp(immediate_op,*op._FUNCptr)){
	      set_abort();
	      tmp=(*op._FUNCptr)(args,contextptr);
	      clear_abort();
	      esc_flag=0;
	      giac::ctrl_c=false;
	      kbd_interrupted=giac::interrupted=false;
	    }
	    else
	      tmp=symbolic(*op._FUNCptr,args);
	    //cout << "sel " << value._EQWptr->g << " " << tmp << " " << goto_sel << endl;
	    esc_flag=0;
	    giac::ctrl_c=false;
	    kbd_interrupted=giac::interrupted=false;
	    if (!is_undef(tmp)){
	      xcas::replace_selection(eq,tmp,gsel,&goto_sel);
	      if (addarg){
		xcas::eqw_select_down(eq.data);
		xcas::eqw_select_leftright(eq,false);
	      }
	      eqdata=xcas::Equation_total_size(eq.data);
	      dx=(eqdata.dx-LCD_WIDTH_PX)/2;
	      dy=LCD_HEIGHT_PX-2*STATUS_AREA_PX+eqdata.y;
	      firstrun=-1; // workaround, force 2 times display
	    }
	  }
	}
      }
    }
  }
  //*logptr(contextptr) << eq.data << endl;
}

bool textedit(char * s){
  if (!s)
    return false;
  int ss=strlen(s);
  if (ss==0){
    *s=' ';
    s[1]=0;
    ss=1;
  }
  textArea ta;
  ta.elements.clear();
  ta.editable=true;
  ta.clipline=-1;
  ta.changed=false;
  ta.filename="\\\\fls0\\temp.py";
  ta.y=0;
  ta.allowEXE=true;
  bool str=s[0]=='"' && s[ss-1]=='"';
  if (str){
    s[ss-1]=0;
    add(&ta,s+1);
  }
  else
    add(&ta,s);
  ta.line=0;
  ta.pos=ta.elements[ta.line].s.size();
  int res=doTextArea(&ta);
  if (res==TEXTAREA_RETURN_EXIT)
    return false;
  string S(merge_area(ta.elements));
  if (str)
    S='"'+S+'"';
  int Ssize=S.size();
  if (Ssize<512){
    strcpy(s,S.c_str());
    for (--Ssize;Ssize>=0;--Ssize){
      if ((unsigned char)s[Ssize]==0x9c || s[Ssize]=='\n')
	s[Ssize]=0;
      if (s[Ssize]!=' ')
	break;
    }
    return true;
  }
  return false;
}

void do_run(const char * s,gen & g,gen & ge){
  if (!contextptr)
    contextptr=new giac::context;
  int S=strlen(s);
  char buf[S+1];
  buf[S]=0;
  for (int i=0;i<S;++i){
    char c=s[i];
    if (c==0x1e || c==char(0x9c))
      buf[i]='\n';
    else {
      if (c==0x0d)
	buf[i]=' ';
      else
	buf[i]=c;
    }
  }
  g=gen(buf,contextptr);
  //Console_Output(g.print(contextptr).c_str()); return ;
  giac::freeze=false;
  execution_in_progress = 1;
  giac::set_abort();
  ge=eval(equaltosto(g,contextptr),1,contextptr);
  giac::clear_abort();
  execution_in_progress = 0;
  if (esc_flag || ctrl_c){
    while (confirm("Interrupted","F1/F6: ok",true)==-1)
      ; // insure ON has been removed from keyboard buffer
    ge=string2gen("Interrupted",false);
    // memory full?
    if (!kbd_interrupted){
      // clear turtle, display msg
      turtle_stack().erase(turtle_stack().begin()+1,turtle_stack().end());// =vector<logo_turtle>(1,logo_turtle());
      while (confirm((lang?"Memoire remplie!":"Memory full"),"Purge variable",true)==-1)
	;
      gen g=select_var();
      if (g.type==_IDNT)
	_purge(g,contextptr);
      else 
	_restart(0,contextptr);
    }
  }
  //Console_Output("Done"); return ;
  esc_flag=0;
  giac::ctrl_c=false;
  kbd_interrupted=giac::interrupted=false;
}

void displaylogo(){
#ifdef TURTLETAB
  xcas::Turtle t={tablogo,0,0,1,1};
#else
  xcas::Turtle t={&turtle_stack(),0,0,1,1};
#endif
  DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
  DisplayStatusArea();
  while (1){
    int save_ymin=clip_ymin;
    clip_ymin=24;
    t.draw();
    clip_ymin=save_ymin;
    int key;
    GetKey(&key);
    if (key==KEY_CTRL_EXIT || key==KEY_PRGM_ACON || key==KEY_CTRL_MENU || key==KEY_CTRL_EXE || key==KEY_CTRL_VARS)
      break;
    if (key==KEY_CTRL_UP){ t.turtley += 10; }
    if (key==KEY_CTRL_PAGEUP) { t.turtley += 100; }
    if (key==KEY_CTRL_DOWN) { t.turtley -= 10; }
    if (key==KEY_CTRL_PAGEDOWN) { t.turtley -= 100;}
    if (key==KEY_CTRL_LEFT) { t.turtlex -= 10; }
    if (key==KEY_SHIFT_LEFT) { t.turtlex -= 100; }
    if (key==KEY_CTRL_RIGHT) { t.turtlex += 10; }
    if (key==KEY_SHIFT_RIGHT) { t.turtlex += 100;}
    if (key==KEY_CHAR_PLUS) { t.turtlezoom *= 2;}
    if (key==KEY_CHAR_MINUS){ t.turtlezoom /= 2;  }
  }  
}

bool stringtodouble(const string & s1,double & d){
  gen g(s1,contextptr);
  g=evalf(g,1,contextptr);
  if (g.type!=_DOUBLE_){
    confirm("Invalid value",s1.c_str());
    return false;
  }
  d=g._DOUBLE_val;
  return true;
}

void displaygraph(const giac::gen & ge){
  // graph display
  //if (aborttimer > 0) { Timer_Stop(aborttimer); Timer_Deinstall(aborttimer);}
  xcas::Graph2d gr(ge);
  gr.show_axes=true;
  // initial setting for x and y
  if (ge.type==_VECT){
    const_iterateur it=ge._VECTptr->begin(),itend=ge._VECTptr->end();
    for (;it!=itend;++it){
      if (it->is_symb_of_sommet(at_equal)){
	const gen & f=it->_SYMBptr->feuille;
	gen & optname = f._VECTptr->front();
	gen & optvalue= f._VECTptr->back();
	if (optname.val==_AXES && optvalue.type==_INT_)
	  gr.show_axes=optvalue.val;
	if (optname.type==_INT_ && optname.subtype == _INT_PLOT && optname.val>=_GL_X && optname.val<=_GL_Z && optvalue.is_symb_of_sommet(at_interval)){
	  //*logptr(contextptr) << optname << " " << optvalue << endl;
	  gen optvf=evalf_double(optvalue._SYMBptr->feuille,1,contextptr);
	  if (optvf.type==_VECT && optvf._VECTptr->size()==2){
	    gen a=optvf._VECTptr->front();
	    gen b=optvf._VECTptr->back();
	    if (a.type==_DOUBLE_ && b.type==_DOUBLE_){
	      switch (optname.val){
	      case _GL_X:
		gr.window_xmin=a._DOUBLE_val;
		gr.window_xmax=b._DOUBLE_val;
		gr.update();
		break;
	      case _GL_Y:
		gr.window_ymin=a._DOUBLE_val;
		gr.window_ymax=b._DOUBLE_val;
		gr.update();
		break;
	      }
	    }
	  }
	}
      }
    }
  }
  // UI
  DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
  EnableStatusArea(2);
  for (;;){
    gr.draw();
    DisplayStatusArea();
    int x=0,y=LCD_HEIGHT_PX-STATUS_AREA_PX-17;
    PrintMini(&x,&y,(unsigned char *)"menu",0x04,0xffffffff,0,0,COLOR_BLACK,COLOR_WHITE,1,0);
    int keyflag = GetSetupSetting( (unsigned int)0x14);
    int key;
    GetKey(&key);
    if (key==KEY_CTRL_F1){
      char menu_xmin[32],menu_xmax[32],menu_ymin[32],menu_ymax[32];
      ustl::string s;
      s="xmin "+print_DOUBLE_(gr.window_xmin,6);
      strcpy(menu_xmin,s.c_str());
      s="xmax "+print_DOUBLE_(gr.window_xmax,6);
      strcpy(menu_xmax,s.c_str());
      s="ymin "+print_DOUBLE_(gr.window_ymin,6);
      strcpy(menu_ymin,s.c_str());
      s="ymax "+print_DOUBLE_(gr.window_ymax,6);
      strcpy(menu_ymax,s.c_str());
      Menu smallmenu;
      smallmenu.numitems=12;
      MenuItem smallmenuitems[smallmenu.numitems];
      smallmenu.items=smallmenuitems;
      smallmenu.height=8;
      //smallmenu.title = "KhiCAS";
      smallmenuitems[0].text = (char *) menu_xmin;
      smallmenuitems[1].text = (char *) menu_xmax;
      smallmenuitems[2].text = (char *) menu_ymin;
      smallmenuitems[3].text = (char *) menu_ymax;
      smallmenuitems[4].text = (char*) "Orthonormalize /";
      smallmenuitems[5].text = (char*) "Autoscale *";
      smallmenuitems[6].text = (char *) ("Zoom in +");
      smallmenuitems[7].text = (char *) ("Zoom out -");
      smallmenuitems[8].text = (char *) ("Y-Zoom out (-)");
      smallmenuitems[9].text = (char*) (lang?"Voir axes":"Show axes");
      smallmenuitems[10].text = (char*) (lang?"Cacher axes":"Hide axes");
      smallmenuitems[11].text = (char*)(lang?"Quitter":"Quit");
      int sres = doMenu(&smallmenu);
      if(sres == MENU_RETURN_SELECTION) {
	const char * ptr=0;
	ustl::string s1; double d;
	if (smallmenu.selection==1){
	  if (inputdouble(menu_xmin,d)){
	    gr.window_xmin=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==2){
	  if (inputdouble(menu_xmax,d)){
	    gr.window_xmax=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==3){
	  if (inputdouble(menu_ymin,d)){
	    gr.window_ymin=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==4){
	  if (inputdouble(menu_ymax,d)){
	    gr.window_ymax=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==5)
	  gr.orthonormalize();
	if (smallmenu.selection==6)
	  gr.autoscale();	
	if (smallmenu.selection==7)
	  gr.zoom(0.7);	
	if (smallmenu.selection==8)
	  gr.zoom(1/0.7);	
	if (smallmenu.selection==9)
	  gr.zoomy(1/0.7);
	if (smallmenu.selection==10)
	  gr.show_axes=true;	
	if (smallmenu.selection==11)
	  gr.show_axes=false;	
	if (smallmenu.selection==12)
	  break;
      }
    }
    if (key==KEY_CTRL_EXIT || key==KEY_PRGM_ACON || key==KEY_CTRL_MENU)
      break;
    if (key==KEY_CTRL_UP){ gr.up((gr.window_ymax-gr.window_ymin)/5); }
    if (key==KEY_CTRL_PAGEUP) { gr.up((gr.window_ymax-gr.window_ymin)/2); }
    if (key==KEY_CTRL_DOWN) { gr.down((gr.window_ymax-gr.window_ymin)/5); }
    if (key==KEY_CTRL_PAGEDOWN) { gr.down((gr.window_ymax-gr.window_ymin)/2);}
    if (key==KEY_CTRL_LEFT) { gr.left((gr.window_xmax-gr.window_xmin)/5); }
    if (key==KEY_SHIFT_LEFT) { gr.left((gr.window_xmax-gr.window_xmin)/2); }
    if (key==KEY_CTRL_RIGHT) { gr.right((gr.window_xmax-gr.window_xmin)/5); }
    if (key==KEY_SHIFT_RIGHT) { gr.right((gr.window_xmax-gr.window_xmin)/5); }
    if (key==KEY_CHAR_PLUS) { gr.zoom(0.7);}
    if (key==KEY_CHAR_MINUS){ gr.zoom(1/0.7); }
    if (key==KEY_CHAR_PMINUS){ gr.zoomy(1/0.7); }
    if (key==KEY_CHAR_MULT){ gr.autoscale(); }
    if (key==KEY_CHAR_DIV) { gr.orthonormalize(); }
    if (key==KEY_CTRL_VARS || key==KEY_CTRL_OPTN) {gr.show_axes=!gr.show_axes;}
  }
  // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
  return ;
}

bool eqws(char * s,bool eval){ // s buffer must be at least 512 char
  gen g,ge;
  int dconsole_save=dconsole_mode;
  int ss=strlen(s);
  for (int i=0;i<ss;++i){
    if (s[i]==char(0x9c))
      s[i]='\n';
  }
  if (ss>=2 && (s[0]=='#' || s[0]=='"' ||
		(s[0]=='/' && (s[1]=='/' || s[1]=='*'))
		))
    return textedit(s);
  dconsole_mode=0;
  if (eval)
    do_run(s,g,ge);
  else {
    if (s[0]==0)
      ge=0;
    else
      ge=gen(s,contextptr);
  }
  dconsole_mode=dconsole_save;
  if (is_undef(ge))
    return textedit(s);
  if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
    if (islogo(ge)){
      displaylogo();
      return false;
    }
    if (ispnt(ge)){
      displaygraph(ge);
      // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
      return false;
    }
    if (ge.is_symb_of_sommet(at_program))
      return textedit(s);
    if (taille(ge,256)>=256)
      return false; // sizeof(eqwdata)=44
  }
  gen tmp=eqw(ge,true);
  if (is_undef(tmp) || tmp==ge || taille(ge,64)>=64)
    return false;
  string S(tmp.print(contextptr));
  if (S.size()>=512)
    return false;
  strcpy(s,S.c_str());
  return true;
}

void check_do_graph(giac::gen & ge,int do_logo_graph_eqw) {
  if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
    if (islogo(ge)){
      if (do_logo_graph_eqw & 4)
	displaylogo();
      return;
    }
    if (ispnt(ge)){
      if (do_logo_graph_eqw & 2)
	displaygraph(ge);
      // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
      return ;
    }
    if ( do_logo_graph_eqw % 2 ==0)
      return;
    if (taille(ge,256)>=256 || ge.is_symb_of_sommet(at_program))
      return ; // sizeof(eqwdata)=44
    gen tmp=eqw(ge,false);
    if (!is_undef(tmp) && tmp!=ge){
      //dConsolePutChar(147);
      Console_Output((const unsigned char *) ge.print(contextptr).c_str());
      Console_NewLine(LINE_TYPE_INPUT, 1);
      ge=tmp;
    }
  }
}

int load_script(const char * filename,ustl::string & s){
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  Bfile_StrToName_ncpy(pFile,(const unsigned char *) filename, strlen(filename)+1); 
  int hFile = Bfile_OpenFile_OS(pFile, READWRITE); // Get handle
  // Check for file existence
  if(hFile < 0) 
    return 0;
  // Returned no error, file exists, open it
  int size = Bfile_GetFileSize_OS(hFile);
  // File exists and has size 'size'
  // Read file into a buffer
  if ((unsigned int)size > MAX_TEXTVIEWER_FILESIZE) {
    Bfile_CloseFile_OS(hFile);
    puts("Stop: script too big");
    return 0; //file too big, return
  }
  unsigned char* asrc = (unsigned char*)alloca(size*sizeof(unsigned char)+5); // 5 more bytes to make sure it fits...
  memset(asrc, 0, size+5); //alloca does not clear the allocated space. Make sure the string is null-terminated this way.
  int rsize = Bfile_ReadFile_OS(hFile, asrc, size, 0);
  Bfile_CloseFile_OS(hFile); //we got file contents, close it
  asrc[rsize]='\0';
  s=string((const char *)asrc);
  return 1;
}

int run_script(const char* filename) {
  // returns 1 if script was run, 0 otherwise
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename)+1); 
  int hFile = Bfile_OpenFile_OS(pFile, READWRITE); // Get handle
  if(hFile < 0) 
    return 0;
  // Check for file existence
  if(hFile >= 0) // Check if it opened
  {
    // Returned no error, file exists, open it
    int size = Bfile_GetFileSize_OS(hFile);
    // File exists and has size 'size'
    // Read file into a buffer
    if ((unsigned int)size > MAX_TEXTVIEWER_FILESIZE) {
      Bfile_CloseFile_OS(hFile);
      puts("Stop: script too big");
      return 0; //file too big, return
    }
    unsigned char* asrc = (unsigned char*)alloca(size*sizeof(unsigned char)+5); // 5 more bytes to make sure it fits...
    memset(asrc, 0, size+5); //alloca does not clear the allocated space. Make sure the string is null-terminated this way.
    int rsize = Bfile_ReadFile_OS(hFile, asrc, size, 0);
    Bfile_CloseFile_OS(hFile); //we got file contents, close it
    asrc[rsize]='\0';
    execution_in_progress = 1;
    run((char*)asrc);
    execution_in_progress = 0;
    if (asrc[0]=='#' || (asrc[0]=='d' && asrc[1]=='e' && asrc[2]=='f' && asrc[3]==' '))
      return 2;
    if ( (asrc[0]=='/' && asrc[1]=='/') ||
	 (rsize>8 && asrc[0]=='f' && (asrc[1]=='o' || asrc[1]=='u') && asrc[2]=='n' && asrc[3]=='c' && asrc[4]=='t' && asrc[5]=='i' && asrc[6]=='o' && asrc[7]=='n' && asrc[8]==' ')
	 )
      return 3;
    return 1;
  }
  return 0;
}

int get_set_session_setting(int value) {
  // value is -1 to get only
  // 0 to disable sesison save/load, 1 to enable
  // if MCS file is present, disable session load/restore
  if(value == -1) {
    int size;
    MCSGetDlen2(DIRNAME, (unsigned char*)SESSIONFILE, &size);
    if (!size) return 1;
    else return 0;
  } else if(value == 1) {
    MCSDelVar2(DIRNAME, (unsigned char*)SESSIONFILE);
    return 1;
  } else if(value == 0) {
    MCS_CreateDirectory(DIRNAME);
    unsigned char buffer[2];
    buffer[0] = 1;
    buffer[1] = 1;
    MCSPutVar2(DIRNAME, (unsigned char*)SESSIONFILE, 2, buffer);
    return 0;
  }
  return -1;
}

#if 0
void create_data_folder() {
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)DATAFOLDER, strlen(DATAFOLDER)+1);
  Bfile_CreateEntry_OS(pFile, CREATEMODE_FOLDER, 0);
}
#endif

string remove_path(const string & st){
  int s=int(st.size()),i;
  for (i=s-1;i>=0;--i){
    if (st[i]=='\\')
      break;
  }
  return st.substr(i+1,s-i-1);
}

void save(const char * fname){
  clear_abort();
  string filename(remove_path(remove_extension(fname)));
  save_console_state_smem(("\\\\fls0\\"+filename+".xw").c_str()); // call before save_khicas_symbols_smem(), because this calls create_data_folder if necessary!
  // save_khicas_symbols_smem(("\\\\fls0\\"+filename+".xw").c_str());
  if (edptr)
    check_leave(edptr);
}

void save_session(){
  if (strcmp(session_filename,"session") && console_changed){
    ustl::string tmp(session_filename);
    tmp += lang?" a ete modifie!":" was modified!";
    if (confirm(tmp.c_str(),lang?"F1: sauvegarder, F6: tant pis":"F1: save, F6: discard changes")==KEY_CTRL_F1){
      save(session_filename);
      console_changed=0;
    }    
  }
  save("session");
  // this is only called on exit, no need to reinstall the check_execution_abort timer.
  if (edptr && edptr->changed && edptr->filename!="\\\\fls0\\session.py"){
    if (!check_leave(edptr)){
      save_script("\\\\fls0\\lastprg.py",merge_area(edptr->elements));
    }
  }
}

int restore_session(const char * fname){
  // cout << "0" << fname << endl; Console_Disp(); GetKey(&key);
  string filename(remove_path(remove_extension(fname)));
  if (!load_console_state_smem((string("\\\\fls0\\")+filename+string(".xw")).c_str())){
    int x=0,y=120;
    PrintMini(&x,&y,(unsigned char*)"KhiCAS 1.5 (c) 2019 B. Parisse et al",0x02, 0xFFFFFFFF, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
    x=0; y=138;
    PrintMini(&x,&y,(unsigned char*)"  License GPL 2",0x02, 0xFFFFFFFF, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
    x=0; y=156;
    PrintMini(&x,&y,(unsigned char*)"  Do not use if CAS is forbidden",0x02, 0xFFFFFFFF, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
    if (confirm("Syntax?","F1: Xcas, F6: Python")==KEY_CTRL_F6)
      python_compat(true,contextptr);
    Bdisp_AllClr_VRAM();  
    //menu_about();
    return 0;
  }
  return 1;
}

int select_script_and_run() {
  char filename[MAX_FILENAME_SIZE+1];
  if(fileBrowser(filename, (char*)"*.py", "Scripts")) 
    return run_script(filename);
  return 0;
}

void erase_script(){
  char filename[MAX_FILENAME_SIZE+1];
  int res=fileBrowser(filename, (char*)"*.py", "Scripts");
  if (res && do_confirm(lang?"Vraiment effacer":"Really erase?")){
    unsigned short pFile[MAX_FILENAME_SIZE+1];
    // create file in data folder (assumes data folder already exists)
    Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename)+1);
    int hFile = Bfile_OpenFile_OS(pFile, READWRITE); // Get handle
    if (hFile>=0){
      Bfile_CloseFile_OS(hFile);
      Bfile_DeleteEntry(pFile);
    }
  }
}

string extract_name(const char * s){
  int l=strlen(s),i,j;
  for (i=l-1;i>=0;--i){
    if (s[i]=='.')
      break;
  }
  if (i<=0)
    return "f";
  for (j=i-1;j>=0;--j){
    if (s[j]=='\\')
      break;
  }
  if (j<0)
    return "f";
  return string(s+j+1).substr(0,i-j-1);
}

void edit_script(char * fname){
  char fname_[MAX_FILENAME_SIZE+1];
  char * filename=0;
  int res=1;
  if (fname)
    filename=fname;
  else {
    res=fileBrowser(fname_, (char*)"*.py", "Scripts");
    filename=fname_;
  }
  if(res) {
    string s;
    load_script(filename,s);
    if (s.empty()){
      s=python_compat(contextptr)?(lang?"Prog. Python, sinon taper":"Python prog., for Xcas"):(lang?"Prog. Xcas, sinon taper":"Xcas prog., for Python");
      s += " AC F6 12";
      int k=confirm(s.c_str(),"F1: Tortue, F6: Prog",true);
      if (k==-1)
	return;
      if (k==KEY_CTRL_F1)
	s="\nefface;\n ";
      else
	s=python_compat(contextptr)?"def "+extract_name(filename)+"(x):\n  \n  return x":"function "+extract_name(filename)+"(x)\nlocal j;\n  \n  return x;\nffunction";
    }
    // split s at newlines
    if (edptr==0)
      edptr=new textArea;
    if (!edptr) return;
    edptr->elements.clear();
    edptr->clipline=-1;
    edptr->filename=filename;
    edptr->editable=true;
    edptr->changed=false;
    edptr->python=python_compat(contextptr);
    edptr->elements.clear();
    add(edptr,s);
    s.clear();
    edptr->line=0;
    //edptr->line=edptr->elements.size()-1;
    edptr->pos=0;
    int res=doTextArea(edptr);
    if (res==-1)
      python_compat(edptr->python,contextptr);
    dConsolePutChar('\x1e');
  }
}

string khicas_state(){
  giac::gen g(giac::_VARS(-1,contextptr)); 
  int b=python_compat(contextptr);
  python_compat(0,contextptr);
#if 1
  char buf[8192]="";
  if (g.type==giac::_VECT){
    for (int i=0;i<g._VECTptr->size();++i){
      string s((*g._VECTptr)[i].print(contextptr));
      if (strlen(buf)+s.size()+128<sizeof(buf)){
	strcat(buf,s.c_str());
	strcat(buf,":;");
      }
    }
  }
  python_compat(b,contextptr);
  if (strlen(buf)+128<sizeof(buf)){
    strcat(buf,"python_compat(");
    strcat(buf,giac::print_INT_(b).c_str());
    strcat(buf,");angle_radian(");
    strcat(buf,angle_radian(contextptr)?"1":"0");
    strcat(buf,");with_sqrt(");
    strcat(buf,withsqrt(contextptr)?"1":"0");
    strcat(buf,");");
  }
  return buf;
#else
  string s(g.print(contextptr));
  python_compat(b,contextptr);
  s += "; python_compat(";
  s +=  giac::print_INT_(b);
  s += ");angle_radian(";
  s += angle_radian(contextptr)?'1':'0';
  s += ");with_sqrt(";
  s += withsqrt(contextptr)?'1':'0';
  s += ");";
  return s;
#endif
}

void save_khicas_symbols_smem(const char * filename) {
  // save variables in xcas mode,
  // because at load time the parser will be in xcas mode
  string s(khicas_state());
  save_script(filename,s);
}


#if 1
int get_custom_key_handler_state() { return 0; }
int get_custom_fkey_label(int fkey) { return 0; }
#else
int get_custom_key_handler_state() {
  U* tmp = usr_symbol("prizmUIhandleKeys");
  if (!issymbol(tmp)) return 0;
  tmp = get_binding(tmp);
  if(isnonnegativeinteger(tmp)) {
    return !iszero(tmp);
  } else return 0;
}
int get_custom_fkey_label(int fkey) {
  U* tmp;
  if(fkey==0) {
    tmp = usr_symbol("prizmUIfkey1label");
  } else if(fkey==1) {
    tmp = usr_symbol("prizmUIfkey2label");
  } else if(fkey==2) {
    tmp = usr_symbol("prizmUIfkey3label");
  } else if (fkey==3) {
    tmp = usr_symbol("prizmUIfkey4label");
  } else if (fkey==5) {
    tmp = usr_symbol("prizmUIfkey6label");
  } else return 0;
  if (issymbol(tmp)) {
    tmp = get_binding(tmp);
    if(isnonnegativeinteger(tmp)) {
      return *tmp->u.q.a;
    }
  }
  return 0;
}
#endif

void printchar(int c){
  dConsolePutChar(c);
}

void printchar_nowrap(int c){
  printchar(c);
}

void printstr(char *s){
  while (*s)
    printchar(*s++);
}

ustl::string last_ans(){
  const vecteur & v=history_out(contextptr);
  if (!v.empty())
    return v.back().print(contextptr);
  return "";
}

void run(const char * s,int do_logo_graph_eqw){
  if (strlen(s)>=2 && (s[0]=='#' ||
		       (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
		       ))
    return;
  gen g,ge;
  do_run(s,g,ge);
  if (giac::freeze){
    giac::freeze=false;
    DefineStatusMessage((char*)(lang?"Ecran fige. Taper EXIT":"Screen freezed. Press EXIT."), 1, 0, 0);
    DisplayStatusArea();
    for (;;){
      int key;
      GetKey(&key);
      if (key==KEY_CTRL_EXIT)
	break;
    }
  }
  int t=giac::taille(g,GIAC_HISTORY_MAX_TAILLE);  
  int te=giac::taille(ge,GIAC_HISTORY_MAX_TAILLE);
  bool do_tex=false;
  if (t<GIAC_HISTORY_MAX_TAILLE && te<GIAC_HISTORY_MAX_TAILLE){
    giac::vecteur &vin=history_in(contextptr);
    giac::vecteur &vout=history_out(contextptr);
    if (vin.size()>GIAC_HISTORY_SIZE)
      vin.erase(vin.begin());
    vin.push_back(g);
    if (vout.size()>GIAC_HISTORY_SIZE)
      vout.erase(vout.begin());
    vout.push_back(ge);
  }
  check_do_graph(ge,do_logo_graph_eqw);
  string s_;
  if (ge.type==giac::_STRNG)
    s_='"'+*ge._STRNGptr+'"';
  else {
    if (te>256)
      s_="Object too large";
    else {
      if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type==giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
	s_="Graphic object";
      else {
	//do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
	// tex support has been disabled!
	s_=ge.print(contextptr);
	// translate to tex? set do_tex to true
      }
    }
  }
  if (s_.size()>512)
    s_=s_.substr(0,509)+"...";
  char* edit_line = (char*)Console_GetEditLine();
  Console_Output((const unsigned char*)s_.c_str());
  //return ge; 
}


