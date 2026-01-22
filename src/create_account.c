/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <mysql/mysql.h>
#include "argon.h"
#include "config.h"

static MYSQL mysql;

int init_database(void) {
    // init database client
    if (!mysql_init(&mysql)) return 0;

    // try to login to database using config data
    if (!mysql_real_connect(&mysql, config_data.dbhost, config_data.dbuser, config_data.dbpass, config_data.dbname, 0, NULL, 0)) {
        fprintf(stderr, "MySQL error: %s (%d)\n", mysql_error(&mysql), mysql_errno(&mysql));
        return 0;
    }

    return 1;
}

void exit_database(void) {
    mysql_close(&mysql);
}

void help(char *prog) {
    fprintf(stderr, "Usage: %s [-s name=value] [-f filename] [-e] <email> <password>\n\n-s Set config name to value (e.g. dbhost=localhost).\n-f Read config file <filename>.\n-e Read configuration from environment variables.\n", prog);
}

int main(int argc, char *args[]) {
    char buf[512];
    char hash[256];
    int c;

    while (1) {
        c = getopt(argc, args, "s:f:e");
        if (c == -1) break;
        switch (c) {
        case 'h':
            help(args[0]);
            exit(0);
        case 's':
            config_string(optarg);
            break;
        case 'f':
            config_file(optarg);
            break;
        case 'e':
            config_getenv();
            break;
        }
    }

    if (argc - optind != 2) {
        help(args[0]);
        return 1;
    }

    if (!init_database()) {
        fprintf(stderr, "Cannot connect to database.\n");
        return 3;
    }

    if (argon2id_hash_password(hash, sizeof(hash), args[optind + 1], NULL)) {
        fprintf(stderr, "Argon failed. Call Mom!\n");
        return 2;
    }

    sprintf(buf, "insert account (email,password,creation_time) values ("
                 "'%s'," // email
                 "'%s'," // password
                 "%d)", // creation time
            args[optind], hash, (int)time(NULL));

    if (mysql_query(&mysql, buf)) {
        fprintf(stderr, "Failed to create account: Error: %s (%d)", mysql_error(&mysql), mysql_errno(&mysql));
        return 2;
    }

    printf("Success. Account ID is %d.\n", (int)mysql_insert_id(&mysql));

    exit_database();

    return 0;
}
