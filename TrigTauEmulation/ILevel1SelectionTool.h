// vim: ts=2 sw=2
#ifndef ILEVEL1SELECTIONTOOL_H
#define ILEVEL1SELECTIONTOOL_H

#include "AsgTools/IAsgTool.h"

class ILevel1SelectionTool : public virtual asg::IAsgTool
{
   ASG_TOOL_INTERFACE(ILevel1SelectionTool)
    
 public:

  virtual ~ILevel1SelectionTool() {};

};

#endif
