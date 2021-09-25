// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -I../include -g -c  symbolic.cc -Wall -DIN_GIAC -DHAVE_CONFIG_H" -*-
#include "giacPCH.h"
/*
 *  Copyright (C) 2000, 2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
using namespace std;
#include "symbolic.h"
#include "identificateur.h"
#include "usual.h"
#include "prog.h"
#include "rpn.h"
#include "plot.h"
#include "giacintl.h"
#include "global.h"

// NB: the operator in the symbolic (sommet) can not be replace by a pointer
// to a unary_function_ptr in the current code. Indeed, symbolic are created
// in the parser from temporary gen objects of type _FUNC which contains
// copies of unary_function_ptr, these copies are deleted once the parser
// gen object is deleted -> segfault.

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // unary_function_ptr quoted_op[]={at_of,at_for,at_bloc,at_local,at_program,at_rpn_prog,at_ifte,at_try_catch,at_print,at_signal,at_as_function_of,at_lieu,at_legende,at_debug,at_sst,at_sst_in,at_cont,at_kill,at_halt,at_watch,at_rmwatch,at_breakpoint,at_maple2mupad,at_mupad2maple,at_maple2xcas,at_mupad2xcas,at_purge,0};
  // functions here must be declared with define_unary_function_eval?_index
  // index is even for on quoted, odd for quoted (1+)
#if defined(GIAC_GENERIC_CONSTANTS) || defined(VISUALC) || defined(x86_64)
  const unary_function_ptr * archive_function_tab(){
    static const unary_function_ptr archive_function_tab_ptr[]={*at_plus,*at_neg,*at_binary_minus,*at_prod,*at_division,*at_inv,*at_pow,*at_exp,*at_ln,*at_abs,*at_arg,*at_pnt,*at_point,*at_segment,*at_sto,*at_sin,
								*at_cos,*at_tan,*at_asin,*at_acos,*at_atan,*at_sinh,*at_cosh,*at_tanh,*at_asinh,*at_acosh,*at_atanh,*at_interval,*at_union,*at_minus,*at_intersect,*at_not,
								*at_and,*at_ou,*at_inferieur_strict,*at_inferieur_egal,*at_superieur_strict,*at_superieur_egal,*at_different,*at_equal,*at_rpn_prog,*at_local,*at_return,*at_Dialog,*at_double_deux_points,*at_pointprod,*at_pointdivision,*at_pointpow,*at_hash,*at_pourcent,*at_tilocal,*at_break,*at_continue,*at_ampersand_times,*at_maple_lib,*at_unit,*at_plot_style,*at_xor,*at_check_type,*at_quote_pow,*at_case,*at_dollar,*at_IFTE,*at_RPN_CASE,*at_RPN_LOCAL,*at_RPN_FOR,*at_RPN_WHILE,*at_NOP,*at_unit,*at_ifte,*at_for,*at_bloc,*at_program,*at_same,*at_increment,*at_decrement,*at_multcrement,*at_divcrement,*at_sq,*at_display,*at_of,*at_at,*at_normalmod,*at_equal2,*at_pointplus,*at_pointminus,*at_struct_dot,*at_try_catch,0};
    archive_function_tab_length=sizeof(archive_function_tab_ptr)/sizeof(const unary_function_ptr *);
    return archive_function_tab_ptr;
  }
  int archive_function_tab_length=0;

#else
  static const size_t archive_function_tab_alias[]={
    // index = 2
    alias_at_plus,alias_at_neg,alias_at_binary_minus,alias_at_prod,alias_at_division,
    // 12
    alias_at_inv,alias_at_pow,alias_at_exp,alias_at_ln,alias_at_abs,
    // 22
    alias_at_arg,alias_at_pnt,alias_at_point,alias_at_segment,alias_at_sto,
    // 32
    alias_at_sin,alias_at_cos,alias_at_tan,alias_at_asin,alias_at_acos,
    // 42
    alias_at_atan,alias_at_sinh,alias_at_cosh,alias_at_tanh,alias_at_asinh,
    // 52
    alias_at_acosh,alias_at_atanh,alias_at_interval,alias_at_union,alias_at_minus,
    // 62
    alias_at_intersect,alias_at_not,alias_at_and,alias_at_ou,alias_at_inferieur_strict,
    // 72
    alias_at_inferieur_egal,alias_at_superieur_strict,alias_at_superieur_egal,alias_at_different,alias_at_equal,
    // 82
    alias_at_rpn_prog,alias_at_local,alias_at_return,alias_at_Dialog,alias_at_double_deux_points,
    // 92
    alias_at_pointprod,alias_at_pointdivision,alias_at_pointpow,alias_at_hash,alias_at_pourcent,
    // 102
    alias_at_tilocal,alias_at_break,alias_at_continue,alias_at_ampersand_times,alias_at_maple_lib,
    // 112
    alias_at_unit,alias_at_plot_style,alias_at_xor,alias_at_check_type,alias_at_quote_pow,
    // 122
    alias_at_case,alias_at_dollar,alias_at_IFTE,alias_at_RPN_CASE,alias_at_RPN_LOCAL,
    // 132
    alias_at_RPN_FOR,alias_at_RPN_WHILE,alias_at_NOP,alias_at_unit,alias_at_ifte,
    // 142
    alias_at_for,alias_at_bloc,alias_at_program,alias_at_same,alias_at_increment,
    // 152
    alias_at_decrement,alias_at_multcrement,alias_at_divcrement,alias_at_sq,alias_at_display,
    // 162
    alias_at_of,alias_at_at,alias_at_normalmod,alias_at_equal2,alias_at_pointplus,
    // 172
    alias_at_pointminus,alias_at_struct_dot,alias_at_try_catch,
    0
  };
  const unary_function_ptr * _archive_function_tab = (const unary_function_ptr *) &archive_function_tab_alias;
  const unary_function_ptr * archive_function_tab(){
    return _archive_function_tab;
  }
  int archive_function_tab_length=sizeof(archive_function_tab_alias)/sizeof(size_t);

#endif

  int archive_function_index(const unary_function_ptr & f){
    return f.ptr()?(f.ptr()->index_quoted_function >> 1):0;
    // return equalposcomp(archive_function_tab(),f);
  }

  int equalposcomp(const unary_function_ptr tab[],const unary_function_ptr & f){
    for (int i=1;*((size_t *)tab)!=0;++tab,++i){
      if (*tab==f)
	return i;
    }
    return 0;
  }

  int equalposcomp(const vector<const unary_function_ptr * > & tab,const unary_function_ptr * f){
    vector<const unary_function_ptr * >::const_iterator it=tab.begin(),itend=tab.end();
    for (int i=1;it!=itend;++it,++i){
      if (**it==*f)
	return i;
    }
    return 0;
  }

  int equalposcomp(const const_unary_function_ptr_ptr tab[],const unary_function_ptr & f){
    for (int i=1;*((size_t *)tab)!=0;++tab,++i){
      if (**tab==f)
	return i;
    }
    return 0;
  }

  symbolic::symbolic(const symbolic & mys,const gen & e): sommet(mys.sommet){
    vecteur tmp;
    if (mys.feuille.type==_VECT){
      tmp = *mys.feuille._VECTptr;
      tmp.push_back(e);
    }
    else {
      tmp.push_back(mys.feuille);
      tmp.push_back(e);
    }
    feuille = gen(tmp,_SEQ__VECT);
  };
  
  symbolic::symbolic(const gen & a,const unary_function_ptr & o,const gen & b):sommet(o) {
    if (b.type==_VECT)
      feuille=gen(mergevecteur(vecteur(1,a),*b._VECTptr),b.subtype);
    else
      feuille=makesequence(a,b);
  };
  
  symbolic::symbolic(const gen & a,const unary_function_ptr * o,const gen & b):sommet(*o) {
    if (b.type==_VECT)
      feuille=gen(mergevecteur(vecteur(1,a),*b._VECTptr),b.subtype);
    else
      feuille=makesequence(a,b);
  };
  
  int symbolic::size() const {
    if (feuille.type==_SYMB)
      return 1+feuille._SYMBptr->size();
    if (feuille.type!=_VECT)
      return 2;
    int s=1;
    iterateur it=feuille._VECTptr->begin(),itend=feuille._VECTptr->end();
    for (;it!=itend;++it){
      if (it->type==_SYMB)
	s += it->_SYMBptr->size();
      else
	++s;
    }
    return s;
  }

  bool print_rewrite_prod_inv=false;

  static string & add_print_plus(string & s,const symbolic & g,GIAC_CONTEXT){
#if 0
    if (is_inf(g.feuille) 
	// && calc_mode(contextptr)!=1
	&& abs_calc_mode(contextptr)==38
	){
      s+="∞";
      return s;
    }
#endif
    if (g.feuille.type!=_VECT){
      s += '+';
      return add_print(s,g.feuille,contextptr);
    }
    vecteur & v = *g.feuille._VECTptr;
    if (v.empty())
      return s;
    int l=int(v.size());
    if (l==1)
      s += '+';
    add_print(s, v.front(),contextptr);
    for (int i=1;i<l;++i){
      s += '+';
      bool par=false;
      if (v[i].type==_SYMB){
	const unary_function_ptr & u=v[i]._SYMBptr->sommet;
	if (u==at_superieur_egal || u==at_superieur_strict || u==at_inferieur_strict || u==at_inferieur_egal)
	  par=true;
      }
      if (par) s+='(';
      add_print(s,v[i],contextptr);
      if (par) s+=')';
    }
    return s;
  }

  static string & add_print_neg(string & s,const gen & feuille,GIAC_CONTEXT){
    int l = int(s.size());
    // +- and -- -> - and +
    if (l && s[l-1]=='+')
      s[l-1]='-';
    else {
      if (l && s[l-1]=='-')
	s[l-1]='+';
      else
	s += calc_mode(contextptr)==38?"−":"-";
    }
    if (feuille.type!=_CPLX){ 
      if(feuille.type!=_SYMB || (feuille._SYMBptr->sommet==at_inv || feuille._SYMBptr->sommet==at_prod) || !need_parenthesis(feuille)){
	return add_print(s,feuille,contextptr);
      }
    }
    s += "(";
    add_print(s,feuille,contextptr); 
    s += ")";
    return s;
  }

  static string & add_print_inv(string & s,const gen & feuille,GIAC_CONTEXT){
    gen f = feuille;
    bool isneg=false;
    bool calc38=calc_mode(contextptr)==38;
    if (f.is_symb_of_sommet(at_neg) && f._SYMBptr->feuille.type!=_VECT){
      f = f._SYMBptr->feuille;
      isneg=true;
    }
    if (f.type<_CPLX && is_positive(-f,contextptr)){
      f=-f;
      isneg=!isneg;
    }
    int l=int(s.size());
    if (isneg && l && s[l-1]=='-'){
      if (l==1) 
	s.clear();
      else
	s[l-1]='+';
      isneg=false;
    }
    if (isneg && l && s[l-1]=='+'){
      s[l-1]='-';
      isneg=false;
    }
    bool bt=f.type==_SYMB && (f._SYMBptr->sommet==at_plus || f._SYMBptr->sommet==at_inv || f._SYMBptr->sommet==at_prod || need_parenthesis(f._SYMBptr->sommet));
    if ( !bt && (f.type!=_CPLX) && (f.type!=_MOD)){
      s += (isneg?(calc38?"−1/":"-1/"):"1/");
      return add_print(s,f,contextptr);
    }
    else {
      s += (isneg?(calc38?"−1/(":"-1"):"1/(");
      add_print(s,f,contextptr);
      s += ")";
      return s;
    }
  }

  static string & add_print_prod(string & s,const symbolic & g,GIAC_CONTEXT){
    gen n0,d0;
    if (print_rewrite_prod_inv &&
	rewrite_prod_inv(g.feuille,n0,d0)
	){
      if (n0.type<_CPLX || n0.type==_IDNT){
	add_print(s,n0,contextptr);
	s += "/";
      }
      else {
	s +="(";
	add_print(s,n0,contextptr);
	s += ")/";
      }
      if (d0.type<_CPLX || d0.type==_IDNT)
	add_print(s,d0,contextptr); // s += d0.print(contextptr);
      else {
	s += "("; 
	add_print(s,d0,contextptr); s += ")"; // s += "("+d0.print(contextptr)+")";
      }
      return s;
    }
    if (g.feuille.type!=_VECT)
      return add_print(s,g.feuille,contextptr);
    vecteur & v = *g.feuille._VECTptr;
    int l=int(v.size());
    for (int i=0;i<l;++i){
      gen e(v[i]);
      if (e.type!=_SYMB){
	if (i)
	  s += '*';
	if ( (e.type==_CPLX 
	      && need_parenthesis(e) 
	      ) || e.type==_MOD ){
	  s += "(";
	  s +=e.print(contextptr);
	  s +=")";
	}
	else 
	  add_print(s,e,contextptr); // s +=e.print(contextptr);
      }
      else {
	if (e._SYMBptr->sommet==at_inv){
	  gen f(e._SYMBptr->feuille);
	  if (i){
	    if ( (f.type==_CPLX) || (f.type==_MOD) ||
		 ((f.type==_SYMB) && 
		  ( (f._SYMBptr->sommet==at_plus) || (f._SYMBptr->sommet==at_prod) || need_parenthesis(f._SYMBptr->sommet) || f._SYMBptr->sommet==at_inv || (f._SYMBptr->sommet==at_pow && f._SYMBptr->feuille[0].is_symb_of_sommet(at_inv))))
		 ){
	      s += "/(";
	      add_print(s,f,contextptr);
	      s += ")"; // s += ("/("+f.print(contextptr) + ")");
	    }
	    else {
	      s += "/";
	      add_print(s,f,contextptr);
	      // s += ("/"+f.print(contextptr));
	    }
	  }
	  else
	    add_print(s,e,contextptr); // s += e.print(contextptr);
	} // end if e._SYMBptr->sommet==at_inv
	else {
	  if (i)
	    s += '*';
	  if ( e._SYMBptr->sommet==at_plus || e._SYMBptr->sommet==at_binary_minus || e._SYMBptr->sommet==at_neg || is_inequation(e) ){
	    s += "(";
	    add_print(s,e,contextptr);
	    s += ")";
	    // s += ("("+e.print(contextptr)+")");
	  }
	  else
	    add_print(s,e,contextptr); // s += e.print(contextptr);
	}
      }
    } // end_for
    return s;
  }

  static string & add_print_pow(string & s,const symbolic & g,GIAC_CONTEXT){
    if (g.feuille.type!=_VECT || g.feuille._VECTptr->size()!=2){
      s += "pow(";
      add_print(s,g.feuille,contextptr);
      s+=')';
      return s;
    }
    gen pui=g.feuille._VECTptr->back();
    gen arg=g.feuille._VECTptr->front();
#ifndef GIAC_HAS_STO_38
    if (__pow.printsommet==&cprintaspow){
      s+="pow(";
      add_print(s,arg,contextptr);
      s+=",";
      add_print(s,pui,contextptr);
      s+=')';
      return s;
    }
#endif
    bool argpar = ( (arg.type>_CPLX && arg.type!=_FLOAT_) || !is_positive(arg,contextptr)) && arg.type!=_IDNT ;
#ifdef EMCC
    bool need=need_parenthesis(arg) || arg.type==_SYMB;
    if (pui==plus_one_half){
      s += (need?"√(":"√");
      add_print(s,arg,contextptr);
      s += (need?")":"");
      return s;
    }
#endif
#if 0
    if (abs_calc_mode(contextptr)==38){
      bool need=need_parenthesis(arg) || arg.type==_SYMB;
      if (pui==plus_one_half){
	s += (need?"√(":"√");
	add_print(s,arg,contextptr);
	s += (need?")":"");
	return s;
      }
      if ( pui==minus_one_half  || pui==fraction(minus_one,plus_two) ){
	s += (need?"1/√(":"1/√");
	add_print(s,arg,contextptr);
	s += (need?")":"");
	return s;
      }
      if (pui==minus_one){
	s += (need?"(":"");
	add_print(s,arg,contextptr);
	s += (need?")":"");
	return s;
	// return s += (need?")\xe2\x81\xb2":"\xe2\x81\xb2");
      }
      if (pui==plus_two){
	s += (argpar?"(":"");
	add_print(s,arg,contextptr);
	s += (argpar?")²":"²");
	return s;
      }
    }
#endif
    if (pui.type>_REAL && pui==plus_one_half){
      s += "sqrt(";
      add_print(s,arg,contextptr);
      s += ')'; 
      return s;
    }
    if ( pui.type>_REAL && (pui==minus_one_half  || pui==fraction(minus_one,plus_two) )){
      s += "1/sqrt(";
      add_print(s,arg,contextptr);
      s += ')';
      return s;
    }
    if (arg.type==_IDNT || (arg.type==_SYMB && arg._SYMBptr->sommet!=at_neg && (arg._SYMBptr->sommet!=at_exp || calc_mode(contextptr)!=1) && !arg._SYMBptr->sommet.ptr()->printsommet)){
      argpar=arg.is_symb_of_sommet(at_inv) || (arg.is_symb_of_sommet(at_exp) && abs_calc_mode(contextptr)==38);
      if (argpar) s +='(';
      if (pui.type==_SYMB || pui.type==_FRAC || pui.type==_CPLX){
	add_print(s,arg,contextptr);
	if (argpar) s +=')';
#ifdef GIAC_HAS_STO_38
	s += '^';
#else
	s += python_compat(contextptr)?"**":__pow.s;
#endif
	s += '(';
	add_print(s,pui,contextptr);
	s += ')';
	return s;
      }
      else {
	add_print(s,arg,contextptr);
	if (argpar) s +=')';
#ifdef GIAC_HAS_STO_38
	s += '^';
#else
	s += python_compat(contextptr)?"**":__pow.s;
#endif
	return add_print(s,pui,contextptr);
      }
    }
    if (argpar)
      s += '(';
    add_print(s,arg,contextptr);
    if (argpar)
      s += ')';
#ifdef GIAC_HAS_STO_38
    s += '^';
#else
    s += python_compat(contextptr)?"**":__pow.s;
#endif
    bool puipar = pui.type==_SYMB || pui.type==_FRAC || pui.type==_CPLX || (pui.type==_VECT && pui.subtype==_SEQ__VECT);
    if (puipar)
      s += '(';
    add_print(s,pui,contextptr);
    if (puipar)
      s += ')';
    return s ;      
  }

  static string & add_print_int(string & s,int i,GIAC_CONTEXT){
    char ch[32];
    int l=int(s.size());
    if (i<0){
      if (l && s[l-1]=='+')
	s[l-1]='-';
      else {
	if (l && s[l-1]=='-'){
	  if (l==1) 
	    s.clear();
	  else
	    s[l-1]='+';
	}
	else
	  s +=  (calc_mode(contextptr)==38)?"−":"-"; // add a minus
      }
      i=-i;
    }
    switch (integer_format(contextptr)){
    case 16:
      my_sprintf(ch,"0x%X",i);
    case 8:
      my_sprintf(ch,"0o%o",i);
    default:
      sprint_int(ch,i);//my_sprintf(ch,"%d",i);
    }
    s += ch;
    return s;
  }

  static string & add_print_symbolic(string & s,const symbolic & g,GIAC_CONTEXT){
    if (!g.sommet.ptr()){
      s+="NULL(";
      s+=g.feuille.print();
      s+=")";
      return s;
    }
    if ( g.sommet.ptr()->printsommet && g.sommet!=at_plus && g.sommet!=at_prod && g.sommet!=at_pow){
      s += g.sommet.ptr()->printsommet(g.feuille,g.sommet.ptr()->s,contextptr);
      return s;
    }
    if ( g.feuille.type==_VECT && g.feuille._VECTptr->empty() ){
      s += g.sommet.ptr()->print(contextptr);
      s += "()";
      return s;
    }
    if (g.sommet==at_prod)
      return add_print_prod(s,g,contextptr);
    if (g.sommet==at_plus)
      return add_print_plus(s,g,contextptr);
    if (g.sommet==at_pow)
      return add_print_pow(s,g,contextptr);
    if (g.sommet==at_neg)
      return add_print_neg(s,g.feuille,contextptr);
    if (g.sommet==at_inv)
      return add_print_inv(s,g.feuille,contextptr);
    if ( g.feuille.type!=_VECT || ( g.sommet!=at_prod && g.feuille._VECTptr->front().type==_VECT ) ){
      s += g.sommet.ptr()->print(contextptr);
      s += '(';
      add_print(s,g.feuille,contextptr);
      s += ')';
      return s;
    }
    int l=int(g.feuille._VECTptr->size());
    s += g.sommet.ptr()->print(contextptr);
    s += '(';
    if (g.feuille.subtype!=_SEQ__VECT)
      s += begin_VECT_string(g.feuille.subtype,false,contextptr);
    for (int i=0;;++i){
      add_print(s,(*(g.feuille._VECTptr))[i],contextptr); // s += (*(feuille._VECTptr))[i].print(contextptr);
      if (i==l-1){
	break;
      }
      s += ',';
    }
    if (g.feuille.subtype!=_SEQ__VECT)
      s += end_VECT_string(g.feuille.subtype,false,contextptr);
    s += ')';
    return s;
  }

  string & add_print(string & s,const gen & g,GIAC_CONTEXT){
    if (g.type==_IDNT){
      s += g._IDNTptr->print(contextptr);
      return s;
    }
    int l=int(s.size());
    if (g.type==_INT_ && g.subtype==0)
      return add_print_int(s,g.val,contextptr);
    if (g.type==_VECT && g.subtype==0){
      s += calc_mode(contextptr)==1?'{':'[';
      add_printinner_VECT(s,*g._VECTptr,0,contextptr);
      s += calc_mode(contextptr)==1?'}':']';
      return s;
    }
    if (g.type==_FRAC && g._FRACptr->num.type==_INT_ && g._FRACptr->den.type==_INT_){
      add_print(s,g._FRACptr->num,contextptr);
      s += "/";
      add_print(s,g._FRACptr->den,contextptr);
      return s;
    }
    if (g.type==_SYMB){
      if (g.subtype==_SPREAD__SYMB)
	s += "=";
      return add_print_symbolic(s,*g._SYMBptr,contextptr);
    }
#ifdef EMCC
    const string tmp=g.print(contextptr);
#else
    const string & tmp=g.print(contextptr);
#endif
    // check +- -> - and -- -> +
    if (l && s[l-1]=='+' ){
      if (!tmp.empty() && tmp[0]=='-'){
	s = s.substr(0,l-1);
	s += tmp;
	return s;
      }
      if (tmp.size()>3 && (unsigned char)tmp[0]==226 && (unsigned char)tmp[1]==136 && (unsigned char) tmp[2]==146 ) { // -30, -120, -110
	s[l-1]='-';
	s += tmp.substr(3,tmp.size()-3);
	return s;
      }
    }
    if (l && s[l-1]=='-' ){
      if (!tmp.empty() && tmp[0]=='-'){
	s[l-1]='+';
	s += tmp.substr(1,tmp.size()-1);
	return s;
      }
      if (tmp.size()>3 && (unsigned char)tmp[0]==226 && (unsigned char)tmp[1]==136 && (unsigned char) tmp[2]==146 ) { // -30, -120, -110
	s[l-1]='+';
	s += tmp.substr(3,tmp.size()-3);
	return s;
      }
    }
    s += tmp;
    return s;
  }

  string symbolic::print(GIAC_CONTEXT) const{
    string s;
    add_print_symbolic(s,*this,contextptr);
    return s;
  }

  /* EVAL without system stack */

  static void eval_sto_pnt_symb(const gen & feuille,gen & e,GIAC_CONTEXT){
    // e is also in history_plot(), ref_count==2
    if (e.type==_SYMB && e.ref_count()<=2 && e._SYMBptr->feuille.type==_VECT && e._SYMBptr->feuille.ref_count()==1){
      vecteur & v=*e._SYMBptr->feuille._VECTptr;
      // legende not converted to string to avoid malloc ->faster
      v.push_back(feuille._VECTptr->back());
      // v=makevecteur(v.front(),v.back(),string2gen(feuille._VECTptr->back().print(contextptr),false));
      e._SYMBptr->feuille.subtype=_PNT__VECT;
    }
    else
      e=(symbolic(at_pnt,gen(makevecteur(e._SYMBptr->feuille._VECTptr->front(),e._SYMBptr->feuille._VECTptr->back(),string2gen(feuille._VECTptr->back().print(contextptr),false)),_PNT__VECT)));
  }

  static void eval_sto_pnt_vect(const gen & feuilleback,gen & e,GIAC_CONTEXT){
    vecteur v=*e._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    gen legende;
    for (int pos=0;it!=itend;++pos,++it){
      if ( (it->type==_SYMB) && (it->_SYMBptr->sommet==at_pnt) && (it->_SYMBptr->feuille._VECTptr->size()==2)){
	if (feuilleback.type==_VECT && pos<int(feuilleback._VECTptr->size()))
	  legende=(*feuilleback._VECTptr)[pos];
	else
	  legende=feuilleback;
	if (legende.type==_IDNT && 0 && abs_calc_mode(contextptr)==38){
	  // HP: if legende is an IDNT, keep it as legende
	  // this save alloc (2 alloc instead of 3 alloc + 1 print
	  *it=(symbolic(at_pnt,gen(makevecteur(it->_SYMBptr->feuille._VECTptr->front(),it->_SYMBptr->feuille._VECTptr->back(),legende),_PNT__VECT)));
	}
	else {
	  *it=(symbolic(at_pnt,gen(makevecteur(it->_SYMBptr->feuille._VECTptr->front(),it->_SYMBptr->feuille._VECTptr->back(),string2gen(legende.print(contextptr),false)),_PNT__VECT)));
	}
      }
    }
    e=gen(v,e.subtype);
  }

  static gen eval_sto(const gen & feuille,int level,GIAC_CONTEXT){ // autoname function
    // detect vector/matrix addressing with () parsed as function definition
    // e.g. M(j,k):=j+k+1 parsed as M:=(j,k)->j+k+1
    // these affectations are marked by a subtype==1 by the parser
    // if destination is a matrix
    vecteur & feuillev=*feuille._VECTptr;
    gen & feuilleback=feuillev.back();
    if (feuille.type==_VECT && feuille.subtype==_SORTED__VECT && feuillev.size()==2 && feuillev.front().is_symb_of_sommet(at_program)){
      gen prog=feuillev.front()._SYMBptr->feuille;
      if (prog.type==_VECT && prog._VECTptr->size()==3 && (prog._VECTptr->front()!=_VECT || prog._VECTptr->front()._VECTptr->size()==2)){
	gen val=prog._VECTptr->back();
	if (feuilleback.type==_IDNT && feuilleback._IDNTptr->eval(1,feuilleback,contextptr).type==_VECT){
	  prog=symb_of(feuilleback,prog._VECTptr->front()); // symbolic(at_of,makesequence(feuilleback,prog._VECTptr->front()));
	  return eval_sto(gen(makevecteur(val,prog),_SORTED__VECT),level,contextptr);
	}
      }
    }
    if ( feuilleback.type==_SYMB && (feuilleback._SYMBptr->sommet==at_unquote || feuilleback._SYMBptr->sommet==at_hash ) ){
      gen ans(_sto(feuille.eval(level,contextptr),contextptr));
      return ans;
    }
    bool quotearg=false;
#ifdef GIAC_HAS_STO_38 // quote STO> E, STO> F, STO>R, STO> X, STO>Y
    if (feuilleback.type==_IDNT){
      const char * ch = feuilleback._IDNTptr->id_name;
      if (strlen(ch)==2 && ch[1]>='0' && ch[1]<='9' && (ch[0]=='E' || ch[0]=='F' || ch[0]=='X' || ch[0]=='Y'))
	quotearg=true;
    }
#endif
    gen e;
    if (quotearg)
      e=feuillev.front();
    else {
      gen * feuillevfront=&feuillev.front();
      // avoid self-modifying code in multi-assign
      if (feuilleback.type==_VECT && feuillevfront->type==_VECT &&feuilleback._VECTptr->size()==feuillevfront->_VECTptr->size()){
	gen tmp;
	e =gen(*feuillevfront->_VECTptr,feuillevfront->subtype);
	iterateur it=e._VECTptr->begin(),itend=e._VECTptr->end();
	for (;it!=itend;++it){
	  if (it->in_eval(level,tmp,contextptr))
	    *it=tmp;
	  else {
	    if (it->type==_VECT)
	      *it=gen(*it->_VECTptr,tmp.subtype);
	  }
	}
      }
      else {
	// e=feuillev.front().eval(level,contextptr);
	if (!feuillevfront->in_eval(level,e,contextptr)){
	  if (feuillevfront->type==_VECT) // avoid self-modifying code
	    e=gen(*feuillevfront->_VECTptr,feuillevfront->subtype);
	  else
	    e=*feuillevfront;
	}
      }
    }
    if (e.type==_SYMB && e._SYMBptr->sommet==at_pnt && e._SYMBptr->feuille.type==_VECT && e._SYMBptr->feuille._VECTptr->size()==2 && (contextptr?!contextptr->previous:!protection_level) )
      eval_sto_pnt_symb(feuille,e,contextptr);
    if ( e.type==_VECT && !e._VECTptr->empty() && e._VECTptr->back().type==_SYMB && e._VECTptr->back()._SYMBptr->sommet==at_pnt && (contextptr?!contextptr->previous:!protection_level))
      eval_sto_pnt_vect(feuilleback,e,contextptr);
    return sto(e,feuilleback,python_compat(contextptr),contextptr);
  } // end sommet==at_sto


  gen symbolic::eval(int level,const context * contextptr) const {
    if (level==0 || !sommet.ptr())
      return *this;
    // FIXME test should be removed later, it's here for tests. See global.cc DEFAULT_EVAL_LEVEL
    int & elevel=eval_level(contextptr);
    if (elevel==26)
      return gensizeerr(contextptr); // too many recursion levels
    if (sommet==at_sto && feuille.type==_VECT)
      return eval_sto(feuille,level,contextptr);
    const char * & last =last_evaled_function_name(contextptr);
    const char * save_last=last;
    last=sommet.ptr()->s;
    const gen * & lastarg=last_evaled_argptr(contextptr);
    const gen * save_lastarg=lastarg;
    lastarg=&feuille;
    gen ans;
    if (sommet.quoted()){ 
#ifndef RTOS_THREADX
      if (feuille.type==_SYMB){ 
	unary_function_ptr & u=feuille._SYMBptr->sommet;
	if (u==at_unquote){
	  ans=sommet(feuille.eval(level,contextptr),contextptr);
	  last=save_last;
	  lastarg=save_lastarg;
	  return ans;
	}
	if (u==at_hash){
	  ans=sommet(gen(*feuille._SYMBptr->feuille._STRNGptr,contextptr),contextptr);
	  last=save_last;
	  lastarg=save_lastarg;
	  return ans;
	}
      }
#endif
      int save_level=elevel;
      elevel=level;
#ifdef NO_STDEXCEPT
      ans=sommet(feuille,contextptr);
#else
      try {
	ans=sommet(feuille,contextptr);
      }
      catch (std::runtime_error & err){
	lastarg=&feuille;
	elevel=save_level;
	throw(err);
      }
#endif
      elevel=save_level;
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    } // if (sommet.quoted())
    else {
      // pnt check required because pnt name might be the identifier->recursion
      if (feuille.type==_VECT && sommet==at_pnt && feuille._VECTptr->size()==3){
	ans=(*sommet.ptr())(gen(vecteur(feuille._VECTptr->begin(),feuille._VECTptr->begin()+2),feuille.subtype).in_eval(level,ans,contextptr)?ans:feuille,contextptr);
	last=save_last;
	lastarg=save_lastarg;
	return ans;
      }
      if (feuille.type==_IDNT && sommet==at_neg && !strcmp(feuille._IDNTptr->id_name,string_infinity)){
	last=save_last;
	lastarg=save_lastarg;
	return minus_inf;
      }
      if (sommet==at_quote){
	last=save_last;
	lastarg=save_lastarg;
	return quote(feuille,contextptr);
      }
      ans=(*sommet.ptr())(feuille.in_eval(level,ans,contextptr)?ans:feuille,contextptr);
      /*
	if (feuille.in_eval(level,ans,contextptr))
	ans=(*sommet.ptr())(ans,contextptr);
	else
	ans=(*sommet.ptr())(feuille,contextptr);
      */
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    }
  }

  bool rewrite_prod_inv(const gen & arg,gen & n,gen & d){
    n=1; d=1;
    if (arg.type==_VECT && !arg._VECTptr->empty() && arg._VECTptr->back().is_symb_of_sommet(at_inv)) {
      vecteur & uv=*arg._VECTptr;
      int tmps=int(uv.size()),invbegin;
      vecteur den(1,uv.back()._SYMBptr->feuille);
      // group all inv from the end to the beginning for the denominator
      for (invbegin=tmps-2;invbegin>=0;--invbegin){
	if (!uv[invbegin].is_symb_of_sommet(at_inv))
	  break;
	den.push_back(uv[invbegin]._SYMBptr->feuille);
      }
      vecteur num;
      for (int i=0;i<=invbegin;++i){
	if (uv[i].is_symb_of_sommet(at_inv) && uv[i]._SYMBptr->feuille.type<_POLY)
	  d=d*uv[i]._SYMBptr->feuille;
	else
	  num.push_back(uv[i]);
      }
      if (!is_one(d))
	den.insert(den.begin(),d);
      if (den.size()>1)
	d=(symb_prod(den));
      else
	d=den.front();
      if (!num.empty()){
	if (num.size()==1)
	  n=num.front();
	else 
	  n=(symb_prod(num));
      }
      return true;
    }
    // Group scalar denominators (warning, do not use for matrices!)
    vecteur num,den;
    prod2frac((symb_prod(arg)),num,den);
    if (!den.empty()){
      if (num.empty())
	n=plus_one;
      else {
	if (num.size()==1)
	  n=num.front();
	else
	  n=(symb_prod(num));
      }
      /* code that does not work with matrices
	 if (den.size()==1)
	 d=den.front();
	 else
	 d=ref_symbolic(symb_prod(den);) 
      */
      if (den.size()==1 && den.front().type<_IDNT){
	d=den.front();
	return true;
      }
    }
    return false;
  }
    
  gen symbolic::evalf(int level,const context * contextptr) const {
    if (level==0)
      return *this;
    const char * & last =last_evaled_function_name(contextptr);
    const char * save_last=last;
    last=sommet.ptr()->s;
    const gen * & lastarg=last_evaled_argptr(contextptr);
    const gen * save_lastarg=lastarg;
    lastarg=&feuille;
    if (sommet==at_sto){ // autoname function
      gen e=feuille._VECTptr->front().evalf(level,contextptr);
      if ((e.type==_SYMB) && (e._SYMBptr->sommet==at_pnt) && (e._SYMBptr->feuille.type==_VECT) && (e._SYMBptr->feuille._VECTptr->size()==2))
	e=(symbolic(at_pnt,gen(makevecteur(e._SYMBptr->feuille._VECTptr->front(),e._SYMBptr->feuille._VECTptr->back(),string2gen(feuille._VECTptr->back().print(contextptr),false)),_PNT__VECT)));
      if ( (e.type==_VECT) && (e._VECTptr->size()) && (e._VECTptr->back().type==_SYMB) && (e._VECTptr->back()._SYMBptr->sommet==at_pnt)){
	vecteur v=*e._VECTptr;
	iterateur it=v.begin(),itend=v.end();
	for (;it!=itend;++it){
	  if ( (it->type==_SYMB) && (it->_SYMBptr->sommet==at_pnt) && (it->_SYMBptr->feuille._VECTptr->size()==2))
	    *it=(symbolic(at_pnt,gen(makevecteur(it->_SYMBptr->feuille._VECTptr->front(),it->_SYMBptr->feuille._VECTptr->back(),string2gen(feuille._VECTptr->back().print(contextptr),false)),_PNT__VECT)));
	}
	e=v;
      }
      last=save_last;
      lastarg=save_lastarg;
      return sto(e,feuille._VECTptr->back(),contextptr);
    }
    gen ans;
    if (sommet==at_plus){
      if (feuille.type!=_VECT){
	if (feuille.type==_IDNT && !strcmp(feuille._IDNTptr->id_name,string_infinity))
	  ans=plus_inf;
	else
	  ans=feuille.evalf(level,contextptr);
      }
      else {
	const_iterateur it=feuille._VECTptr->begin(),itend=feuille._VECTptr->end();
	for (;it!=itend;++it){
	  ans=ans+it->evalf(level,contextptr);
	}
      }
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    }
    if (sommet==at_prod){
      if (feuille.type!=_VECT)
	ans=feuille.evalf(level,contextptr);
      else {
	ans=1;
	const_iterateur it=feuille._VECTptr->begin(),itend=feuille._VECTptr->end();
	for (;it!=itend;++it){
	  ans=operator_times(ans,it->evalf(level,contextptr),contextptr);
	}
      }
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    }
    if (sommet.quoted() && sommet!=at_and && !equalposcomp(plot_sommets,sommet) ){ 
      ans=sommet(feuille,contextptr);
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    }
    else {
      if ((sommet==at_neg) && (feuille.type==_IDNT) && !strcmp(feuille._IDNTptr->id_name,string_infinity)){
	last=save_last;
	lastarg=save_lastarg;
	return minus_inf;
      }
      if (sommet==at_quote){
	last=save_last;
	lastarg=save_lastarg;
	return quote(feuille,contextptr);
      }
      if (sommet==at_and || (equalposcomp(plot_sommets,sommet))){
	// bool save_is_inevalf=is_inevalf;
	// is_inevalf=true;
	ans=(symbolic(sommet,feuille.evalf(1,contextptr)));
	// is_inevalf=save_is_inevalf;
	last=save_last;
	lastarg=save_lastarg;
	return ans;
      }
      ans=(*sommet.ptr())(feuille.evalf(level,contextptr),contextptr);
      last=save_last;
      lastarg=save_lastarg;
      return ans;
    }
  }


  unsigned taille(const gen & g,unsigned max){
    if (g.type<=_IDNT)
      return 1;
    if (g.type==_FRAC)
      return 1+taille(g._FRACptr->num,max)+taille(g._FRACptr->den,max);
    if (g.type==_SYMB){
      return 1+taille(g._SYMBptr->feuille,max);
    }
    if (g.type==_VECT){
      unsigned res=0;
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	res += taille(*it,max);
	if (max && res>max)
	  return res;
      }
      return res;
    }
    return 2;
  }

  int malloc_size(int s,int nalloc=2){
    return s+nalloc*8;
  }

  void tailles(const gen & g,vector<int> & v){
    switch (g.type){
    case _INT_: case _DOUBLE_: case _FLOAT_: case _FUNC:
      ++v[0];
      return;
    case _CPLX:
      tailles(*g._CPLXptr,v);
      tailles(*(g._CPLXptr+1),v);
      ++v[1];
      v[8] += malloc_size(sizeof(ref_complex));
      return;
    case _IDNT:
      ++v[2];
      // v[8] += malloc_size(sizeof(ref_identificateur)); // shared in syms()
      return;
    case _FRAC:
      tailles(g._FRACptr->num,v);
      tailles(g._FRACptr->den,v);
      ++v[3];
      v[8] += malloc_size(sizeof(ref_fraction));
      return;
    case _VECT: {
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      ++v[4];
      //CERR << g._VECTptr->capacity() << " "  << itend-it  << " " << g << endl;
      v[8] += malloc_size(sizeof(ref_vecteur))+g._VECTptr->capacity()*sizeof(gen);
      for (;it!=itend;++it){
	tailles(*it,v);
      }
      return;
    }
    case _SYMB:
      tailles(g._SYMBptr->feuille,v);
      ++v[5];
      v[8] += malloc_size(sizeof(ref_symbolic));
      return;
    case _STRNG:
      v[8] += g._STRNGptr->capacity() + malloc_size(sizeof(ref_string));
      ++v[6] ;
      return;
    default:
      ++v[7];
    }
  }

  int bytesize(const gen & g){
    vector<int> v(9);
    tailles(g,v);
    return v[8];
  }
  
  int print_max_depth=100;
  unsigned depth(const gen & g,unsigned add,unsigned max){
    gen g_(g);
    for (;g_.type==_SYMB;++add){
      g_=g_._SYMBptr->feuille;
    }
    if (add>=max)
      return add;
    if (g_.type==_VECT){
      unsigned res=add;
      const_iterateur it=g_._VECTptr->begin(),itend=g_._VECTptr->end();
      for (;it!=itend;++it){
	unsigned cur=depth(*it,add,max);
	if (max && cur>max)
	  return res;
	if (cur>res)
	  res=cur;
      }
      return res;
    }
    return add;
  }

#ifdef NSPIRE
  template<class T> nio::ios_base<T> & operator << (nio::ios_base<T> & os,const symbolic & s) { return os << s.print(context0); }
#else    
  ostream & operator << (ostream & os,const symbolic & s) { return os << s.print(context0); }
#endif

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
