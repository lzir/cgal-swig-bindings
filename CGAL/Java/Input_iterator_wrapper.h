#ifndef CGAL_SWIG_JAVA_INPUT_ITERATOR_WRAPPER_H
#define CGAL_SWIG_JAVA_INPUT_ITERATOR_WRAPPER_H

#include <boost/iterator/iterator_facade.hpp>
#include "global_functions.h"

template <class Cpp_wrapper,class Cpp_base>
class Input_iterator_wrapper:
public boost::iterator_facade<
    Input_iterator_wrapper<Cpp_wrapper,Cpp_base>,
    Cpp_base,
    boost::single_pass_traversal_tag,
    typename boost::mpl::if_<
          boost::mpl::bool_<internal::Converter<Cpp_wrapper>::is_reference>, 
          const Cpp_base&, Cpp_base
        >::type //this allows to use a reference for dereference when possible
    >
{
  friend class boost::iterator_core_access;
  std::string signature;
  const jobject* jiterator; //we suppose that it is not garbaged collected:
                            //the original must stay valid while iterator used.
                            //Usually ok because used within the same function
  Cpp_wrapper* current_ptr;
  jmethodID getCPtr_id, next_id, hasnext_id;
  jclass it_class,pt_class;
  
  void update_with_next_point(bool first=false){
    if (static_cast<bool> (JNU_GetEnv()->CallBooleanMethod(*jiterator,hasnext_id)) )
    {
      jobject jpoint=JNU_GetEnv()->CallObjectMethod(*jiterator,next_id);
      if (first){
        pt_class = JNU_GetEnv()->GetObjectClass(jpoint);
        assert(pt_class!=NULL);
        getCPtr_id=JNU_GetEnv()->GetStaticMethodID(pt_class, "getCPtr",signature.c_str());
      }
      assert(getCPtr_id!=NULL);
      assert(pt_class!=NULL);
      jlong jpt=(jlong) JNU_GetEnv()->CallStaticObjectMethod(pt_class,getCPtr_id,jpoint);
      current_ptr = (Cpp_wrapper*) jpt;
      
    }
    else{
      current_ptr=NULL;
    }
  }
  
public:
  

  Input_iterator_wrapper():current_ptr(NULL){}
  Input_iterator_wrapper(const jobject& jiterator_,const char* sign):signature(sign),jiterator(&jiterator_)
  {
    it_class=JNU_GetEnv()->GetObjectClass(*jiterator);
    assert(it_class!=NULL);
    hasnext_id=JNU_GetEnv()->GetMethodID(it_class, "hasNext", "()Z");
    assert(hasnext_id!=NULL);
    next_id=JNU_GetEnv()->GetMethodID(it_class, "next","()Ljava/lang/Object;");
    getCPtr_id=NULL;
    pt_class=NULL;
    assert(next_id!=NULL);
    update_with_next_point(true);
  }

  
  void increment(){assert(current_ptr!=NULL); update_with_next_point();}
  bool equal(const Input_iterator_wrapper & other) const{ return current_ptr==other.current_ptr; }
  
  typename boost::mpl::if_<
        boost::mpl::bool_<internal::Converter<Cpp_wrapper>::is_reference>, 
        const Cpp_base&, Cpp_base
      >::type
    dereference() const { return internal::Converter<Cpp_wrapper>::convert(*current_ptr); }
};



#endif// CGAL_SWIG_JAVA_INPUT_ITERATOR_WRAPPER_H