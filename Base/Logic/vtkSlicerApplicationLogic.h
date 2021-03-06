/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
///  vtkSlicerApplicationLogic - the main logic to manage the application
///
/// The Main entry point for the slicer3 application.
/// -- manages the connection to the mrml scene
/// -- manages the creation of Views and Slices (logic only)
/// -- serves as central point for dispatching events
/// There is a corresponding vtkSlicerApplicationGUI class that provides
/// a user interface to this class by observing this class.
//

#ifndef __vtkSlicerApplicationLogic_h
#define __vtkSlicerApplicationLogic_h

// Slicer includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

// ITK includes
#include <itkMultiThreader.h>
#include <itkMutexLock.h>

class vtkMRMLSelectionNode;
class vtkMRMLInteractionNode;
class vtkSlicerTask;
class ModifiedQueue;
class ProcessingTaskQueue;
class ReadDataQueue;
class ReadDataRequest;
class WriteDataQueue;
class WriteDataRequest;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerApplicationLogic
  : public vtkMRMLApplicationLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerApplicationLogic *New();
  vtkTypeRevisionMacro(vtkSlicerApplicationLogic, vtkMRMLApplicationLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Perform the default behaviour related to selecting a fiducial list
  /// (display it in the Fiducials GUI)
  void PropagateFiducialListSelection();

  /// Create a thread for processing
  void CreateProcessingThread();

  /// Shutdown the processing thread
  void TerminateProcessingThread();
  enum RequestEvents
    {
      RequestModifiedEvent = vtkCommand::UserEvent,
      RequestReadDataEvent,
      RequestWriteDataEvent
    };

  /// Schedule a task to run in the processing thread. Returns true if
  /// task was successfully scheduled. ScheduleTask() is called from the
  /// main thread to run something in the processing thread.
  int ScheduleTask( vtkSlicerTask* );

  /// Request a Modified call on an object.  This method allows a
  /// processing thread to request a Modified call on an object to be
  /// performed in the main thread.  This allows the call to Modified
  /// to trigger GUI changes. RequestModified() is called from the
  /// processing thread to modify an object in the main thread.
  int RequestModified( vtkObject * );

  /// Request that data be read from a file and set it on the referenced
  /// node.  The request will be sent to the main thread which will be
  /// responsible for reading the data, setting it on the referenced
  /// node, and updating the display.
  int RequestReadData(const char *refNode, const char *filename,
                       int displayData = false,
                       int deleteFile=false);

  /// Return the number of items that need to be read from the queue
  /// (this allows code that invokes command line modules to know when
  /// multiple items are being returned and have all been returned).
  unsigned int GetReadDataQueueSize();


  /// Request that data be written from a file to a remote destination.
  int RequestWriteData(const char *refNode, const char *filename,
                       int displayData = false,
                       int deleteFile=false);

  /// Request that a scene be read from a file. Mappings of node IDs in
  /// the file (sourceIDs) to node IDs in the main scene
  /// (targetIDs) can be specified. Only nodes listed in sourceIDs are
  /// loaded back into the main scene.  Hierarchical nodes will be
  /// handled specially, in that only the top node needs to be listed
  /// in the sourceIds.
  int RequestReadScene(const std::string& filename,
                       std::vector<std::string> &targetIDs,
                       std::vector<std::string> &sourceIDs,
                       int displayData = false,
                       int deleteFile = false);

  /// Process a request on the Modified queue.  This method is called
  /// in the main thread of the application because calls to Modified()
  /// can cause an update to the GUI. (Method needs to be public to fit
  /// in the event callback chain.)
  void ProcessModified();

  /// Process a request to read data and set it on a referenced node.
  /// This method is called in the main thread of the application
  /// because calls to load data will cause a Modified() on a node
  /// which can force a render.
  void ProcessReadData();

  /// Process a request to write data from a referenced node.
  void ProcessWriteData();

  /// These routings act as place holders so that test scripts can
  /// turn on and off tracing.  These are just hooks
  /// for use with external tracing tool (such as AQTime)
  void SetTracingOn () { this->Tracing = 1; }
  void SetTracingOff () { this->Tracing = 0; }

  /// Return True if \a filePath isn't contained in \a applicationHomeDir.
  static bool IsExtension(const std::string& filePath, const std::string& applicationHomeDir);

  /// Return \a true if the plugin identified with its \a filePath is loaded from an install tree.
  /// \warning Since internally the function looks for the existence of CMakeCache.txt, it will
  /// return an incorrect result if the plugin is installed in the build tree of
  /// an other project.
  static bool IsPluginInstalled(const std::string& filePath, const std::string& applicationHomeDir);

  /// Get share directory associated with \a moduleName located in \a filePath
  static std::string GetModuleShareDirectory(const std::string& moduleName, const std::string& filePath);

  /// Get Slicer-X.Y share directory associated with module located in \a filePath
  static std::string GetModuleSlicerXYShareDirectory(const std::string& filePath);

  /// Get Slicer-X.Y lib directory associated with module located in \a filePath
  static std::string GetModuleSlicerXYLibDirectory(const std::string& filePath);

protected:

  vtkSlicerApplicationLogic();
  ~vtkSlicerApplicationLogic();

  /// Callback used by a MultiThreader to start a processing thread
  static ITK_THREAD_RETURN_TYPE ProcessingThreaderCallback( void * );

  /// Callback used by a MultiThreader to start a networking thread
  static ITK_THREAD_RETURN_TYPE NetworkingThreaderCallback( void * );

  /// Task processing loop that is run in the processing thread
  void ProcessProcessingTasks();

  /// Networking Task processing loop that is run in a networking thread
  void ProcessNetworkingTasks();

  /// Process a request to read data into a node.  This method is
  /// called by ProcessReadData() in the application main thread
  /// because calls to load data will cause a Modified() on a node
  /// which can force a render.
  void ProcessReadNodeData( ReadDataRequest &req );
  void ProcessWriteNodeData( WriteDataRequest &req );

  /// Process a request to read data into a scene.  This method is
  /// called by ProcessReadData() in the application main thread
  /// because calls to load data will cause a Modified() on a node
  /// which can force a render.
  void ProcessReadSceneData( ReadDataRequest &req );
  void ProcessWriteSceneData( WriteDataRequest &req );

private:
  vtkSlicerApplicationLogic(const vtkSlicerApplicationLogic&);
  void operator=(const vtkSlicerApplicationLogic&);

  itk::MultiThreader::Pointer ProcessingThreader;
  itk::MutexLock::Pointer ProcessingThreadActiveLock;
  itk::MutexLock::Pointer ProcessingTaskQueueLock;
  itk::MutexLock::Pointer ModifiedQueueActiveLock;
  itk::MutexLock::Pointer ModifiedQueueLock;
  itk::MutexLock::Pointer ReadDataQueueActiveLock;
  itk::MutexLock::Pointer ReadDataQueueLock;
  itk::MutexLock::Pointer WriteDataQueueActiveLock;
  itk::MutexLock::Pointer WriteDataQueueLock;
  int ProcessingThreadId;
  std::vector<int> NetworkingThreadIDs;
  int ProcessingThreadActive;
  int ModifiedQueueActive;
  int ReadDataQueueActive;
  int WriteDataQueueActive;

  ProcessingTaskQueue* InternalTaskQueue;
  ModifiedQueue*       InternalModifiedQueue;
  ReadDataQueue*       InternalReadDataQueue;
  WriteDataQueue*      InternalWriteDataQueue;

  /// For use with external tracing tool (such as AQTime)
  int Tracing;
};

#endif
