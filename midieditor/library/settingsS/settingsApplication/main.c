#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <libconfig.h>
#include <stdbool.h>
#include "settings.h"
#include "stb_ds.h"

extern const char *appName = "additive";

int main(int argc, char**argv)
{
    bool succ;
    fprintf(stderr, "greger is %d\n", loadInt("greger", &succ));
    saveInt("afafafa", 47);
    return 0;
    char path[PATH_MAX] = {};
    char* home = getenv("HOME");
    assert(home);
    strncat(path, home, PATH_MAX);
    strncat(path, "/.config/", PATH_MAX);
    struct stat sb;
    if(stat(path, &sb) != 0 || (! S_ISDIR(sb.st_mode))) {
        abort();
    }
    strncat(path, appName, PATH_MAX);
    if(stat(path, &sb) != 0 || (! S_ISDIR(sb.st_mode))) {
        mkdir(path, 0777);
    }
    strncat(path, "/", PATH_MAX);
    strncat(path, appName, PATH_MAX);
    strncat(path, ".conf", PATH_MAX);


    config_t cfg;
    config_setting_t *setting, *setting2;
    const char *str;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
//    if(! config_read_file(&cfg, path))
//    {
//        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
//                config_error_line(&cfg), config_error_text(&cfg));
//        config_destroy(&cfg);
//        return(EXIT_FAILURE);
//    }
//    config_read_file(&cfg, path);

//    setting = config_root_setting(&cfg);
    /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, path))
    {
      fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
              config_error_line(&cfg), config_error_text(&cfg));
      config_destroy(&cfg);
      return(EXIT_FAILURE);
    }

    char* ss;
    int res = config_lookup_string(&cfg, "title", &ss);
    if(res == CONFIG_TRUE) {
        fprintf(stderr, ss);
    } else {
        fprintf(stderr, "no read ^((");
    }
    /* Get the store name. */
    if(config_lookup_string(&cfg, "name", &str))
      printf("Store name: %s\n\n", str);
    else
      fprintf(stderr, "No 'name' setting in configuration file.\n");

    /* Output a list of all books in the inventory. */
    setting = config_lookup(&cfg, "inventory.books");
    if(setting != NULL)
    {
      int count = config_setting_length(setting);
      int i;

      printf("%-30s  %-30s   %-6s  %s\n", "TITLE", "AUTHOR", "PRICE", "QTY");

      for(i = 0; i < count; ++i)
      {
        config_setting_t *book = config_setting_get_elem(setting, i);

        /* Only output the record if all of the expected fields are present. */
        const char *title, *author;
        double price;
        int qty;

        if(!(config_setting_lookup_string(book, "title", &title)
             && config_setting_lookup_string(book, "author", &author)
             && config_setting_lookup_float(book, "price", &price)
             && config_setting_lookup_int(book, "qty", &qty)))
          continue;

        printf("%-30s  %-30s  $%6.2f  %3d\n", title, author, price, qty);
      }
      putchar('\n');
    }

    /* Output a list of all movies in the inventory. */
    setting = config_lookup(&cfg, "inventory.movies");
    if(setting != NULL)
    {
      unsigned int count = config_setting_length(setting);
      unsigned int i;

      printf("%-30s  %-10s   %-6s  %s\n", "TITLE", "MEDIA", "PRICE", "QTY");
      for(i = 0; i < count; ++i)
      {
        config_setting_t *movie = config_setting_get_elem(setting, i);

        /* Only output the record if all of the expected fields are present. */
        const char *title, *media;
        double price;
        int qty;

        if(!(config_setting_lookup_string(movie, "title", &title)
             && config_setting_lookup_string(movie, "media", &media)
             && config_setting_lookup_float(movie, "price", &price)
             && config_setting_lookup_int(movie, "qty", &qty)))
          continue;

        printf("%-30s  %-10s  $%6.2f  %3d\n", title, media, price, qty);
      }
      putchar('\n');
    }

    config_destroy(&cfg);
    return(EXIT_SUCCESS);

    return 0;
    setting2 = config_setting_add(setting, "title", CONFIG_TYPE_STRING);
//    int res = config_setting_get_string(&setting, "title", &ss);
    config_setting_set_string(setting2, "Buckaroo Banzai");

    if(! config_write_file(&cfg, path))
    {
        fprintf(stderr, "Error while writing file.\n");
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
    fprintf(stderr, "wrote to %s\n", path);

//    config_write_file(&cfg, path);
    config_destroy(&cfg);

    printf("Hello World!\n");
    return 0;
}
