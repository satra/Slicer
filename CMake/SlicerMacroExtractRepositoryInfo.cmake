################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# The following CMake macro will attempt to 'guess' which type of repository is 
# associated with the current source directory. Then, base on the type of repository, 
# it will extract its associated information.
#

#
# SlicerMacroExtractRepositoryInfo(VAR_PREFIX <var-prefix> [SOURCE_DIR <dir>])
# 
# If no SOURCE_DIR is provided, it will default to CMAKE_CURRENT_SOURCE_DIR.
#
# The macro will define the following variables:
#  <var-prefix>_WC_TYPE - Either 'git' or 'svn' - The type will also be 'svn' if 'git-svn' is used.
#
# If a GIT repository is associated with SOURCE_DIR, the macro 
# will define the following variables:
#  <var-prefix>_WC_URL - url of the repository (at SOURCE_DIR)
#  <var-prefix>_WC_REVISION_NAME - url of the repository (at SOURCE_DIR)
#  <var-prefix>_WC_REVISION_HASH - current revision
#  <var-prefix>_WC_REVISION - Equal to <var-prefix>_WC_REVISION_HASH if not a git-svn repository
#
# If a SVN or GIT-SVN repository is associated with SOURCE_DIR, the macro 
# will define the following variables:
#  <var-prefix>_WC_URL - url of the repository (at SOURCE_DIR)
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO 
#

MACRO(SlicerMacroExtractRepositoryInfo)
  SET(options)
  SET(oneValueArgs VAR_PREFIX SOURCE_DIR)
  SET(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  # Sanity checks
  if("${MY_VAR_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "error: VAR_PREFIX should be specified !")
  endif()
  
  SET(wc_info_prefix ${MY_VAR_PREFIX})
  
  if ("${MY_SOURCE_DIR}" STREQUAL "")
    SET(MY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  
  # Clear variables
  SET(${wc_info_prefix}_WC_TYPE)
  SET(${wc_info_prefix}_WC_INFO)

  # Clear SVN, GIT_SVN specific variables
  SET(${wc_info_prefix}_WC_URL "NA")
  SET(${wc_info_prefix}_WC_REVISION 0)
  SET(${wc_info_prefix}_WC_LAST_CHANGED_AUTHOR)
  SET(${wc_info_prefix}_WC_LAST_CHANGED_DATE)
  SET(${wc_info_prefix}_WC_LAST_CHANGED_REV)
  # Clear GIT specific variables
  SET(${wc_info_prefix}_WC_REVISION_NAME "NA")
  SET(${wc_info_prefix}_WC_REVISION_HASH "NA")
  
  FIND_PACKAGE(Git REQUIRED)
  
  # Is <SOURCE_DIR> a git working copy ?
  EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list -n 1 HEAD
    WORKING_DIRECTORY ${MY_SOURCE_DIR}
    RESULT_VARIABLE GIT_result
    OUTPUT_QUIET
    ERROR_QUIET)
  
  IF(${GIT_result} EQUAL 0)
  
    SET(${wc_info_prefix}_WC_TYPE git)
    GIT_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} ${wc_info_prefix})
    IF(${wc_info_prefix}_WC_GITSVN)
      SET(${wc_info_prefix}_WC_TYPE svn)
    ENDIF()
    
  ELSE()
  
    FIND_PACKAGE(Subversion REQUIRED)
    
    # Is <SOURCE_DIR> a svn working copy ?
    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info
      WORKING_DIRECTORY ${MY_SOURCE_DIR}
      RESULT_VARIABLE Subversion_result
      OUTPUT_QUIET
      ERROR_QUIET)
    
    IF(${Subversion_result} EQUAL 0)
    
      SET(${wc_info_prefix}_WC_TYPE svn)
      Subversion_WC_INFO(${MY_SOURCE_DIR} ${wc_info_prefix})
      
    ELSE()
    
      FIND_PACKAGE(CVS REQUIRED)
      
      # Is <SOURCE_DIR> a CVS working copy ?
      IF(EXISTS ${MY_SOURCE_DIR}/CVS)
        
        # TODO
        MESSAGE(AUTHOR_WARNING "CVS info extraction not yet implemented !")
      
      ELSE()
      
        MESSAGE(WARNING "warning: Failed to extract repository info associated with SOURCE_DIR:${MY_SOURCE_DIR}")
        
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()

