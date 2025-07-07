#ifndef DRIVER_MENU_BACKEND_H__
#define DRIVER_MENU_BACKEND_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct menu_file_list_cbs
{
   int (*action_deferred_push)(void *data, void *userdata, const char
         *path, const char *label, unsigned type);
   int (*action_ok)(const char *path, const char *label, unsigned type,
         size_t idx);
   int (*action_start)(unsigned type,  const char *label, unsigned action);
   int (*action_toggle)(unsigned type, const char *label, unsigned action);
} menu_file_list_cbs_t;

typedef struct menu_ctx_driver_backend
{
   int      (*iterate)(unsigned);
   unsigned (*type_is)(const char *, unsigned);
   void     (*setting_set_label)(char *, size_t, unsigned *,
         unsigned, const char *, const char *, unsigned);
   void  (*list_insert)(void *, const char *, const char *, unsigned, size_t);
   void  (*list_delete)(void *, size_t, size_t);
   void  (*list_clear)(void *);
   void  (*list_set_selection)(void *);
   const char *ident;
} menu_ctx_driver_backend_t;

#ifdef __cplusplus
}
#endif

#endif
