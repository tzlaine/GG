##########################################################################
# Boost Utilities                                                        #
##########################################################################
# Copyright (C) 2007 Douglas Gregor <doug.gregor@gmail.com>              #
# Copyright (C) 2007 Troy Straszheim                                     #
#                                                                        #
# Distributed under the Boost Software License, Version 1.0.             #
# See accompanying file LICENSE_1_0.txt or copy at                       #
#   http://www.boost.org/LICENSE_1_0.txt                                 #
##########################################################################
# Macros in this module:                                                 #
#                                                                        #
#   list_contains: Determine whether a string value is in a list.        #
#                                                                        #
#   car: Return the first element in a list                              #
#                                                                        #
#   cdr: Return all but the first element in a list                      #
#                                                                        #
#   parse_arguments: Parse keyword arguments for use in other macros.    #
##########################################################################

function (pretty_print list)
    if (${list})
        string(REPLACE ";" "\n  " pretty_list "${${list}}")
    else ()
        set(pretty_list "[EMPTY]")
    endif ()
    message("${list}=\n  ${pretty_list}")
endfunction ()

# This utility macro determines whether a particular string value
# occurs within a list of strings:
#
#  list_contains(result string_to_find arg1 arg2 arg3 ... argn)
# 
# This macro sets the variable named by result equal to TRUE if
# string_to_find is found anywhere in the following arguments.
macro (list_contains var value)
  set(${var})
  foreach (value2 ${ARGN})
    if (${value} STREQUAL ${value2})
      set(${var} TRUE)
    endif (${value} STREQUAL ${value2})
  endforeach (value2)
endmacro ()

# This macro is an internal utility macro that updates compilation and
# linking flags based on interactions among the features in a variant.
#
#   feature_interactions(prefix
#                        feature1 feature2 ...)
#
# where "prefix" is the prefix of the compilation and linking flags
# that will be updated (e.g., ${prefix}_COMPILE_FLAGS). feature1,
# feature2, etc. are the names of the features used in this particular
# variant. If the features in this variant conflict, set
# ${prefix}_OKAY to FALSE.
macro (feature_interactions PREFIX)
  # Don't build or link against a shared library and a static run-time
  list_contains(IS_SHARED SHARED ${ARGN})
  list_contains(IS_STATIC STATIC ${ARGN})
  if (IS_SHARED AND IS_STATIC)
    set(${PREFIX}_OKAY FALSE)
  endif (IS_SHARED AND IS_STATIC)

  # Visual C++-specific runtime library flags; with Visual C++, the dynamic
  # runtime is multi-threaded only
  if (MSVC)
    list_contains(IS_SINGLE_THREADED SINGLE_THREADED ${ARGN})
    if (IS_SHARED AND IS_SINGLE_THREADED)
      set(${PREFIX}_OKAY FALSE)
    endif (IS_SHARED AND IS_SINGLE_THREADED) 

    list_contains(IS_DEBUG DEBUG ${ARGN})
    if (IS_DEBUG)
      if (IS_STATIC)
        set(${PREFIX}_COMPILE_FLAGS "/MTd ${${PREFIX}_COMPILE_FLAGS}")
      else (IS_STATIC)
        set(${PREFIX}_COMPILE_FLAGS "/MDd ${${PREFIX}_COMPILE_FLAGS}")
      endif (IS_STATIC)       
    else (IS_DEBUG)
      if (IS_STATIC)
        set(${PREFIX}_COMPILE_FLAGS "/MT ${${PREFIX}_COMPILE_FLAGS}")
      else (IS_STATIC)
        set(${PREFIX}_COMPILE_FLAGS "/MD ${${PREFIX}_COMPILE_FLAGS}")
      endif (IS_STATIC)       
    endif (IS_DEBUG)
  endif (MSVC)  
endmacro ()

# This macro is an internal utility macro that builds a particular variant of
# a library.
#
#   library_variant(libname 
#                   feature1 feature2 ...)
#
# where libname is the name of the library (e.g., "GiGiSDL") and feature1,
# feature2, ... are the features that will be used in this variant.
#
# This macro will define a new library target based on libname and the
# specific variant name, which depends on the utility target libname. The
# compilation and linking flags for this library are defined by
# THIS_LIB_COMPILE_FLAGS, THIS_LIB_LINK_FLAGS, THIS_LIB_LINK_LIBS, and all of
# the compile and linking flags implied by the features provided.
#
# If any of the features listed conflict with this library, no new targets
# will be built.
macro (library_variant LIBNAME)
  set(THIS_VARIANT_COMPILE_FLAGS "${THIS_LIB_COMPILE_FLAGS}")
  set(THIS_VARIANT_LINK_FLAGS "${THIS_LIB_LINK_FLAGS}")
  set(THIS_VARIANT_LINK_LIBS ${THIS_LIB_LINK_LIBS})

  # Determine if it is okay to build this variant
  set(THIS_VARIANT_OKAY TRUE)
  foreach (ARG ${ARGN})
    # If the user specified that we should not build any variants of
    # this kind, don't. For example, if the BUILD_SHARED option is
    # off, don't build shared libraries.
    if (NOT BUILD_${ARG})
      set(THIS_VARIANT_OKAY FALSE)
    endif (NOT BUILD_${ARG})

    # Accumulate compile and link flags
    set(THIS_VARIANT_COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS} ${THIS_LIB_${ARG}_COMPILE_FLAGS} ${${ARG}_COMPILE_FLAGS}")
    set(THIS_VARIANT_LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS} ${THIS_LIB_${ARG}_LINK_FLAGS} ${${ARG}_LINK_FLAGS}")
    set(THIS_VARIANT_LINK_LIBS ${THIS_VARIANT_LINK_LIBS} ${THIS_LIB_${ARG}_LINK_LIBS} ${${ARG}_LINK_LIBS})
  endforeach (ARG ${ARGN})

  # Handle feature interactions
  feature_interactions(THIS_VARIANT ${ARGN})

  if (THIS_VARIANT_OKAY)
    if (IS_STATIC)
      set(VARIANT_LIBNAME ${LIBNAME}_static)
    else ()
      set(VARIANT_LIBNAME ${LIBNAME})
    endif ()

    # We handle static vs. dynamic libraries differently
    list_contains(THIS_LIB_IS_STATIC STATIC ${ARGN})
    if (THIS_LIB_IS_STATIC)
      # On Windows, we need static and shared libraries to have
      # different names, so we prepend "lib" to the name.
      if (WIN32 AND NOT CYGWIN)
        set(LIBPREFIX "lib")
      else (WIN32 AND NOT CYGWIN)
        set(LIBPREFIX "")
      endif (WIN32 AND NOT CYGWIN)

      # Add the library itself
      add_library(${VARIANT_LIBNAME} STATIC ${THIS_LIB_SOURCES})

      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME "${LIBPREFIX}${LIBNAME}"
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LINK_SEARCH_END_STATIC true
        LABELS "${PROJECT_NAME}"
        )
    elseif (THIS_LIB_MODULE)
      # Add a module
      add_library(${VARIANT_LIBNAME} MODULE ${THIS_LIB_SOURCES})

      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME ${LIBNAME}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LABELS "${PROJECT_NAME}"
        PREFIX ""
       # SOVERSION "${BOOST_VERSION}"
        )
    else (THIS_LIB_IS_STATIC)
      #TODO: Check the SOVERSION behavior on Linux and Windows
      # Add a module
      add_library(${VARIANT_LIBNAME} SHARED ${THIS_LIB_SOURCES})
      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME ${LIBNAME}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LABELS "${PROJECT_NAME}"
        # SOVERSION "${BOOST_VERSION}"
        )
    endif (THIS_LIB_IS_STATIC)

    # Link against whatever libraries this library depends on
    target_link_libraries(${VARIANT_LIBNAME} ${THIS_VARIANT_LINK_LIBS})
    foreach (dependency ${THIS_LIB_DEPENDS})
      target_link_libraries(${VARIANT_LIBNAME} "${dependency}")
    endforeach (dependency)

#    # Setup installation properties
#    string(TOLOWER "${PROJECT_NAME}" LIB_COMPONENT)
#    string(REPLACE "-" "_" LIB_COMPONENT ${LIB_COMPONENT})
#
#    # Installation of this library variant
#    string(TOLOWER ${PROJECT_NAME} libname)
#    install(TARGETS ${VARIANT_LIBNAME} DESTINATION lib COMPONENT ${LIB_COMPONENT})
#    set_property( 
#          TARGET ${VARIANT_LIBNAME}
#          PROPERTY BOOST_CPACK_COMPONENT
#          ${LIB_COMPONENT})
#
#    # Make the library installation component dependent on the library
#    # installation components of dependent libraries.
#    set(THIS_LIB_COMPONENT_DEPENDS)
#    foreach (DEP ${THIS_LIB_DEPENDS})
#      # We ask the library variant that this library depends on to tell us
#      # what it's associated installation component is. We depend on that 
#      # installation component.
#      get_property(DEP_COMPONENT 
#        TARGET "${DEP}"
#        PROPERTY BOOST_CPACK_COMPONENT)
#
#      if (DEP_COMPONENT)
#        if (DEP_COMPONENT STREQUAL LIB_COMPONENT)
#          # Do nothing: we have library dependencies within one 
#          # Boost library
#        else ()
#          list(APPEND THIS_LIB_COMPONENT_DEPENDS ${DEP_COMPONENT})
#        endif ()
#      endif ()
#    endforeach (DEP)
#
#    if (COMMAND cpack_add_component)
#      cpack_add_component(${LIB_COMPONENT}
#        DISPLAY_NAME "${VARIANT_DISPLAY_NAME}"
#        GROUP ${libname}
#        DEPENDS ${THIS_LIB_COMPONENT_DEPENDS})
#    endif ()
  endif ()
endmacro ()

macro (library_all_variants LIBNAME)
    library_variant(${LIBNAME} STATIC DEBUG MULTI_THREADED)
    library_variant(${LIBNAME} STATIC DEBUG SINGLE_THREADED)
    library_variant(${LIBNAME} STATIC RELEASE MULTI_THREADED)
    library_variant(${LIBNAME} STATIC RELEASE SINGLE_THREADED)
    library_variant(${LIBNAME} SHARED DEBUG MULTI_THREADED)
    library_variant(${LIBNAME} SHARED DEBUG SINGLE_THREADED)
    library_variant(${LIBNAME} SHARED RELEASE MULTI_THREADED)
    library_variant(${LIBNAME} SHARED RELEASE SINGLE_THREADED)
endmacro ()
