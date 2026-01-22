/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>
#define EXTERNAL_PROGRAM
#include "server.h"
#undef EXTERNAL_PROGRAM
#include "drdata.h"
#include "config.h"

static MYSQL mysql;

int init_database(void) {
    // init database client
    if (!mysql_init(&mysql)) return 0;

    // try to login as root with our password
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
    fprintf(stderr, "Usage: %s [-s name=value] [-f filename] [-e] <accountID> <name> <genderandclassandgod>\ngenderandclassandgod: MW = male warrior, FM = female mage, FWG = female warrior god\n-s Set config name to value (e.g. dbhost=localhost).\n-f Read config file <filename>.\n-e Read configuration from environment variables.\n", prog);
}

int create_char(int user_ID, char *new_user, char *class) {
    char data[80];
    unsigned long long flag = 0;
    int mirror;
    char buf[4096], dbuf[1024], ddata[1024];

    if (class[2] == 'G') flag |= CF_GOD;

    if (class[1] == 'W') flag |= CF_WARRIOR;
    else flag |= CF_MAGE;

    if (class[0] == 'M') flag |= CF_MALE;
    else flag |= CF_FEMALE;

    mysql_real_escape_string(&mysql, data, (char *)&flag, sizeof(flag));

    *(unsigned int *)(ddata + 0) = DRD_JUNK_PPD;
    *(unsigned int *)(ddata + 4) = 16;
    memset(ddata + 8, 0, 16);

    mysql_real_escape_string(&mysql, dbuf, ddata, 4 + 4 + 16);

    mirror = RANDOM(26) + 1;

    sprintf(buf, "insert chars values ("
                 "0," // ID
                 "'%s'," // name
                 "%u," // class (aka flags)
                 "%d," // sID
                 "0," // karma
                 "0," // clan
                 "0," // clan rank
                 "0," // clan serial
                 "0," // experience
                 "0," // current_area
                 "1," // allowed_area
                 "%d," // creation_time
                 "1," // login_time
                 "1," // logout_time
                 "'N'," // locked
                 "'%s'," // chr
                 "'%s'," // item
                 "'%s'," // ppd
                 "%d," // mirror
                 "0)", // current mirror
            new_user,
            (unsigned int)(flag & 0xffffffff),
            user_ID,
            (int)time(NULL),
            data,
            data,
            dbuf,
            mirror);

    if (mysql_query(&mysql, buf)) {
        if (mysql_errno(&mysql) == ER_DUP_ENTRY) {
            fprintf(stderr, "Sorry, the name %s is already taken.\n", new_user);
            return 1;
        } else {
            fprintf(stderr, "Failed to create account %s: Error: %s (%d) (%s)\n", new_user, mysql_error(&mysql), mysql_errno(&mysql), buf);
            return 2;
        }
    }

    return 0;
}

int main(int argc, char **args) {
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

    if (argc - optind != 3) {
        help(args[0]);
        return 1;
    }

    if (!init_database()) {
        fprintf(stderr, "Cannot connect to database.\n");
        return 3;
    }

    if (!create_char(atoi(args[optind]), args[optind + 1], args[optind + 2])) {
        printf("Success.\n");
    }

    exit_database();

    return 0;
}
