/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLThreeDViewDisplayableManagerFactory.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLThreeDViewDisplayableManagerGroup.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInstantiator.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLThreeDViewDisplayableManagerFactory, "$Revision: 13859 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkMRMLThreeDViewDisplayableManagerFactory);

//----------------------------------------------------------------------------
class vtkMRMLThreeDViewDisplayableManagerFactory::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Manager classNames
  std::vector<std::string> DisplayableManagerClassNames;

  // .. and its associated convenient typedef
  typedef std::vector<std::string>::iterator DisplayableManagerClassNamesIt;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::vtkInternal::vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkMRMLThreeDViewDisplayableManagerFactory methods

//----------------------------------------------------------------------------
/// Default initialization to zero is necessary.
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::Instance;

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::classInitialize()
{
  // Allocate the singleton
  Self::Instance = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::classFinalize()
{
  Self::Instance->Delete();
  Self::Instance = 0;
}

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::New()
{
  vtkMRMLThreeDViewDisplayableManagerFactory* instance = Self::GetInstance();
  instance->Register(0);
  return instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()
{
  if(!Self::Instance)
    {
    // Try the factory first
    Self::Instance = (vtkMRMLThreeDViewDisplayableManagerFactory*)vtkObjectFactory::CreateInstance(
        "vtkMRMLThreeDViewDisplayableManagerFactory");

    // if the factory did not provide one, then create it here
    if(!Self::Instance)
      {
      // if the factory failed to create the object,
      // then destroy it now, as vtkDebugLeaks::ConstructClass was called
      // with "vtkMRMLThreeDViewDisplayableManagerFactory", and not the real name of the class
#ifdef VTK_DEBUG_LEAKS
      vtkDebugLeaks::DestructClass("vtkMRMLThreeDViewDisplayableManagerFactory");
#endif
      Self::Instance = new vtkMRMLThreeDViewDisplayableManagerFactory;
      }
    }
  // return the instance
  return Self::Instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::vtkMRMLThreeDViewDisplayableManagerFactory()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::~vtkMRMLThreeDViewDisplayableManagerFactory()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLThreeDViewDisplayableManagerFactory::IsDisplayableManagerRegistered(const char* vtkClassName)
{
  // Sanity checks
  if (!vtkClassName)
    {
    vtkWarningMacro(<<"IsDisplayableManagerRegistered - vtkClassName is NULL");
    return false;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    return false;
    }
  else
    {
    return true;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::RegisterDisplayableManager(const char* vtkClassName)
{
  // Sanity checks
  if (!vtkClassName)
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - vtkClassName is NULL");
    return;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassName);

  if ( it != this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - " << vtkClassName << " already registered");
    return;
    }

  // Check if vtkClassName is a valid vtk className
  vtkSmartPointer<vtkObject> objectPointer;
  objectPointer.TakeReference(vtkInstantiator::CreateInstance(vtkClassName));
  if (!objectPointer)
    {
    return;
    }

  // Register it
  this->Internal->DisplayableManagerClassNames.push_back(vtkClassName);

  this->InvokeEvent(Self::DisplayableManagerFactoryRegisteredEvent,
                    const_cast<char*>(vtkClassName));
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::UnRegisterDisplayableManager(const char* vtkClassName)
{
  // Sanity checks
  if (!vtkClassName)
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - vtkClassName is NULL");
    return;
    }
  
  // Check if the DisplayableManager is registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - " << vtkClassName << " is NOT registered");
    return;
    }

  this->Internal->DisplayableManagerClassNames.erase(it);

  this->InvokeEvent(Self::DisplayableManagerFactoryUnRegisteredEvent,
                    const_cast<char*>(vtkClassName));
}

//----------------------------------------------------------------------------
int vtkMRMLThreeDViewDisplayableManagerFactory::GetRegisteredDisplayableManagerCount()
{
  return this->Internal->DisplayableManagerClassNames.size();
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerGroup* vtkMRMLThreeDViewDisplayableManagerFactory::InstantiateDisplayableManagers(
    vtkRenderer * newRenderer)
{
  // Sanity checks
  if (!newRenderer)
    {
    vtkWarningMacro(<<"InstanciateDisplayableManagers - newRenderer is NULL");
    return 0;
    }

  vtkMRMLThreeDViewDisplayableManagerGroup * displayableManagerGroup = vtkMRMLThreeDViewDisplayableManagerGroup::New();

  for(std::size_t i=0; i < this->Internal->DisplayableManagerClassNames.size(); ++i)
    {
    const char* className = this->Internal->DisplayableManagerClassNames[i].c_str();

    // Object will be unregistered when the SmartPointer will go out-of-scope
    vtkSmartPointer<vtkObject> objectSmartPointer;
    objectSmartPointer.TakeReference(vtkInstantiator::CreateInstance(className));
    vtkMRMLAbstractThreeDViewDisplayableManager* displayableManager =
        vtkMRMLAbstractThreeDViewDisplayableManager::SafeDownCast(objectSmartPointer);
    if (!displayableManager)
      {
      vtkErrorMacro(<<"InstantiateDisplayableManagers - Failed to instantiate " << className);
      continue;
      }

    // A Group observes the factory and eventually instantiates new DisplayableManager
    // when they are registered in the factory
    displayableManagerGroup->SetAndObserveDisplayableManagerFactory(this);

    // Note that DisplayableManagerGroup will take ownership of the object
    displayableManagerGroup->AddAndInitialize(displayableManager);
    }

  displayableManagerGroup->Initialize(newRenderer);

  return displayableManagerGroup;
}


//

//----------------------------------------------------------------------------
// vtkMRMLThreeDViewDisplayableManagerFactoryInitialize methods

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactoryInitialize::vtkMRMLThreeDViewDisplayableManagerFactoryInitialize()
{
  if(++Self::Count == 1)
    { vtkMRMLThreeDViewDisplayableManagerFactory::classInitialize(); }
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactoryInitialize::~vtkMRMLThreeDViewDisplayableManagerFactoryInitialize()
{
  if(--Self::Count == 0)
    { vtkMRMLThreeDViewDisplayableManagerFactory::classFinalize(); }
}

//----------------------------------------------------------------------------
/// Default initialization to zero is necessary.
unsigned int vtkMRMLThreeDViewDisplayableManagerFactoryInitialize::Count;
