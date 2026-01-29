
struct config_data {
    char *dbhost;
    char *dbuser;
    char *dbpass;
    char *dbname;
    char *chathost;
    char *svrkey;
};
extern struct config_data config_data;

void config_string(char *buf);
void config_file(char *file, int isfailok);
void config_getenv(void);
