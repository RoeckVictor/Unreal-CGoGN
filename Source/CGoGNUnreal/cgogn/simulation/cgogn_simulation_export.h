
#ifndef CGOGN_SIMULATION_EXPORT_H
#define CGOGN_SIMULATION_EXPORT_H

#ifdef CGOGN_SIMULATION_STATIC_DEFINE
#  define CGOGN_SIMULATION_EXPORT
#  define CGOGN_SIMULATION_NO_EXPORT
#else
#  ifndef CGOGN_SIMULATION_EXPORT
#    ifdef cgogn_simulation_EXPORTS
        /* We are building this library */
#      define CGOGN_SIMULATION_EXPORT 
#    else
        /* We are using this library */
#      define CGOGN_SIMULATION_EXPORT 
#    endif
#  endif

#  ifndef CGOGN_SIMULATION_NO_EXPORT
#    define CGOGN_SIMULATION_NO_EXPORT 
#  endif
#endif

#ifndef CGOGN_SIMULATION_DEPRECATED
#  define CGOGN_SIMULATION_DEPRECATED __declspec(deprecated)
#endif

#ifndef CGOGN_SIMULATION_DEPRECATED_EXPORT
#  define CGOGN_SIMULATION_DEPRECATED_EXPORT CGOGN_SIMULATION_EXPORT CGOGN_SIMULATION_DEPRECATED
#endif

#ifndef CGOGN_SIMULATION_DEPRECATED_NO_EXPORT
#  define CGOGN_SIMULATION_DEPRECATED_NO_EXPORT CGOGN_SIMULATION_NO_EXPORT CGOGN_SIMULATION_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CGOGN_SIMULATION_NO_DEPRECATED
#    define CGOGN_SIMULATION_NO_DEPRECATED
#  endif
#endif

#endif /* CGOGN_SIMULATION_EXPORT_H */
