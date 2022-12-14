
#ifndef CGOGN_MODELING_EXPORT_H
#define CGOGN_MODELING_EXPORT_H

#ifdef CGOGN_MODELING_STATIC_DEFINE
#  define CGOGN_MODELING_EXPORT
#  define CGOGN_MODELING_NO_EXPORT
#else
#  ifndef CGOGN_MODELING_EXPORT
#    ifdef cgogn_modeling_EXPORTS
        /* We are building this library */
#      define CGOGN_MODELING_EXPORT 
#    else
        /* We are using this library */
#      define CGOGN_MODELING_EXPORT 
#    endif
#  endif

#  ifndef CGOGN_MODELING_NO_EXPORT
#    define CGOGN_MODELING_NO_EXPORT 
#  endif
#endif

#ifndef CGOGN_MODELING_DEPRECATED
#  define CGOGN_MODELING_DEPRECATED __declspec(deprecated)
#endif

#ifndef CGOGN_MODELING_DEPRECATED_EXPORT
#  define CGOGN_MODELING_DEPRECATED_EXPORT CGOGN_MODELING_EXPORT CGOGN_MODELING_DEPRECATED
#endif

#ifndef CGOGN_MODELING_DEPRECATED_NO_EXPORT
#  define CGOGN_MODELING_DEPRECATED_NO_EXPORT CGOGN_MODELING_NO_EXPORT CGOGN_MODELING_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CGOGN_MODELING_NO_DEPRECATED
#    define CGOGN_MODELING_NO_DEPRECATED
#  endif
#endif

#endif /* CGOGN_MODELING_EXPORT_H */
