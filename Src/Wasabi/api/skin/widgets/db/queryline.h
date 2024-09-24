#ifndef _QUERYLINE_H
#define _QUERYLINE_H

#include <api/skin/nakedobject.h>
#include <api/db/subqueryserver.h>

#define QUERYLINE_PARENT NakedObject

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class QueryLine : public QUERYLINE_PARENT, public SubQueryServerI {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  QueryLine(const char *query=NULL);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~QueryLine() { }

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void setQuery(const char *query);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int setAuto(int bv);

protected:
  int autoquery;

private:
  String querytext, autofield;
};

#endif
