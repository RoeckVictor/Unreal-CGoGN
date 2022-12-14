
#ifndef CGOGN_GEOMETRY_EXPORT_H
#define CGOGN_GEOMETRY_EXPORT_H

#ifdef CGOGN_GEOMETRY_STATIC_DEFINE
#  define CGOGN_GEOMETRY_EXPORT
#  define CGOGN_GEOMETRY_NO_EXPORT
#else
#  ifndef CGOGN_GEOMETRY_EXPORT
#    ifdef cgogn_geometry_EXPORTS
        /* We are building this library */
#      define CGOGN_GEOMETRY_EXPORT 
#    else
        /* We are using this library */
#      define CGOGN_GEOMETRY_EXPORT 
#    endif
#  endif

#  ifndef CGOGN_GEOMETRY_NO_EXPORT
#    define CGOGN_GEOMETRY_NO_EXPORT 
#  endif
#endif

#ifndef CGOGN_GEOMETRY_DEPRECATED
#  define CGOGN_GEOMETRY_DEPRECATED __declspec(deprecated)
#endif

#ifndef CGOGN_GEOMETRY_DEPRECATED_EXPORT
#  define CGOGN_GEOMETRY_DEPRECATED_EXPORT CGOGN_GEOMETRY_EXPORT CGOGN_GEOMETRY_DEPRECATED
#endif

#ifndef CGOGN_GEOMETRY_DEPRECATED_NO_EXPORT
#  define CGOGN_GEOMETRY_DEPRECATED_NO_EXPORT CGOGN_GEOMETRY_NO_EXPORT CGOGN_GEOMETRY_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CGOGN_GEOMETRY_NO_DEPRECATED
#    define CGOGN_GEOMETRY_NO_DEPRECATED
#  endif
#endif

#endif /* CGOGN_GEOMETRY_EXPORT_H */
