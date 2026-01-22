-- Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.

CREATE DATABASE /*!32312 IF NOT EXISTS*/ merc35;

USE merc35;

CREATE TABLE area (
  ID int(11) NOT NULL,
  mirror int(11) not null default '0',
  name varchar(80) not null,
  players int(11) not null default '0',
  alive_time int(11) not null default '0',
  idle int(11) not null default '0',
  server int(10) unsigned not null default '0',
  port int(11) not null default '0',
  bps int(11) NOT NULL default '0',
  mem_usage int(11) NOT NULL default '0',
  last_error int(11) NOT NULL default '0',
  UNIQUE KEY name (name),
  KEY ID (ID),
  KEY mirror (mirror)
);


CREATE TABLE badip (
  IP int(10) unsigned NOT NULL default '0',
  t int(11) NOT NULL default '0',
  KEY lookup (IP,t),
  KEY t (t)
);


CREATE TABLE badname (
  ID int(11) NOT NULL auto_increment,
  bad varchar(40) NOT NULL,
  PRIMARY KEY  (ID),
  UNIQUE KEY bad (bad)
);


CREATE TABLE chars (
  ID int(11) NOT NULL auto_increment,
  name varchar(40) NOT NULL,
  class int(11) not null default '0',
  sID int(11) not null default '0',
  karma int(11) not null default '0',
  clan int(11) not null default '0',
  clan_rank int(11) not null default '0',
  clan_serial int(11) not null default '0',
  experience int(11) not null default '0',
  current_area int(11) not null default '0',
  allowed_area int(11) not null default '0',
  creation_time int(11) not null default '0',
  login_time int(11) not null default '0',
  logout_time int(11) not null default '0',
  locked enum('Y','N') default 'N',  
  chr blob default NULL,
  item blob default NULL,
  ppd blob default NULL,
  mirror int(11) not null default '0',
  current_mirror int(11) not null default '0',
  PRIMARY KEY  (ID),
  UNIQUE KEY name (name),
  KEY experience (experience),
  KEY current_area (current_area),
  KEY sID (sID),
  KEY clan (clan)
);


CREATE TABLE clanlog (
  ID int(11) NOT NULL auto_increment,
  time int(11) NOT NULL default '0',
  clan int(11) NOT NULL default '0',
  serial int(11) NOT NULL default '0',
  cID int(11) NOT NULL default '0',
  prio int(11) NOT NULL default '0',
  content text NOT NULL,
  PRIMARY KEY  (ID),
  KEY time (time),
  KEY clan (clan),
  KEY cID (cID),
  KEY prio (prio)
);


CREATE TABLE clubs (
  ID int(11) NOT NULL default '0',
  name varchar(80) NOT NULL,
  paid int(11) NOT NULL default '0',
  money int(11) NOT NULL default '0',
  serial int(11) NOT NULL default '0',
  PRIMARY KEY  (ID)
);


CREATE TABLE constants (
  name varchar(8) NOT NULL,
  val int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (name)
);


CREATE TABLE ipban (
  ID int(11) NOT NULL auto_increment,
  ip int(10) unsigned NOT NULL default '0',
  created int(11) NOT NULL default '0',
  valid int(11) NOT NULL default '0',
  sID int(11) NOT NULL default '0',
  PRIMARY KEY  (ID),
  KEY ip (ip),
  KEY created (created),
  KEY valid (valid)
);


CREATE TABLE iplog (
  ID int(11) NOT NULL auto_increment,
  ip int(10) unsigned not null default '0',
  use_time int(11) not null default '0',
  use_count int(11) not null default '0',
  sID int(11) not null default '0',
  PRIMARY KEY  (ID),
  KEY ip (ip),
  KEY use_time (use_time),
  KEY use_count (use_count),
  KEY sID (sID)
);


CREATE TABLE notes (
  ID int(11) NOT NULL auto_increment,
  uID int(11) NOT NULL default '0',
  kind int(11) not null default '0',
  cID int(11) not null default '0',
  date int(11) not null default '0',
  content blob not null,
  PRIMARY KEY  (ID),
  KEY kind (kind),
  KEY cID (cID),
  KEY uID (uID),
  KEY date (date)
);


CREATE TABLE stats (
  name varchar(10) NOT NULL,
  value int(11) NOT NULL default '0',
  PRIMARY KEY  (name)
);


CREATE TABLE storage (
  ID int(11) NOT NULL default '0',
  description varchar(80) not null,
  version int(11) not null default '0',
  content blob not null,
  PRIMARY KEY  (ID)
);


CREATE TABLE account (
  ID int(11) NOT NULL auto_increment,
  email varchar(80) not null,
  password varchar(255) not null,
  creation_time int(11) default '0',
  banned enum('Y','N','I') default 'I',
  karma int not null default '0',
  banned_till int not null default '0',
  stat_state enum('Created','Char','Logged','Lvl10','Lvl20','Lvl30','Lvl40','Lvl50','Lvl60','Lvl70','Lvl80') NOT NULL default 'Created',
  login_time int(11) NOT NULL default '0',
  PRIMARY KEY (ID),
  KEY email (email),
  KEY creation_time (creation_time)
);


CREATE TABLE task (
  ID int(11) NOT NULL auto_increment,
  content blob not null,
  PRIMARY KEY  (ID)
);


CREATE TABLE createip (
  IP int(10) unsigned NOT NULL default '0',
  t int(11) NOT NULL default '0',
  KEY lookup (IP,t),
  KEY t (t)
);


CREATE TABLE sub_note (
  ID int(11) NOT NULL auto_increment,
  sID int(11) NOT NULL default '0',
  kind int(11) NOT NULL default '0',
  date int(11) NOT NULL default '0',
  note text NOT NULL,
  PRIMARY KEY  (ID),
  KEY sID (sID),
  KEY x1 (sID,kind),
  KEY date (date)
);


CREATE TABLE summary (
  name varchar(10) NOT NULL,
  accounts_new int(11) NOT NULL default '0',
  accounts_with_char int(11) NOT NULL default '0',
  accounts_logged_in int(11) NOT NULL default '0',
  accounts_level_10 int(11) NOT NULL default '0',
  accounts_level_20 int(11) NOT NULL default '0',
  accounts_level_30 int(11) NOT NULL default '0',
  accounts_level_40 int(11) NOT NULL default '0',
  accounts_level_50 int(11) NOT NULL default '0',
  accounts_level_60 int(11) NOT NULL default '0',
  accounts_level_70 int(11) NOT NULL default '0',
  accounts_level_80 int(11) NOT NULL default '0',
  unpaid_time int(11) NOT NULL default '0',
  unpaid_cnt int(11) NOT NULL default '0',
  complaints int(11) NOT NULL default '0',
  warnings int(11) NOT NULL default '0',
  punishments int(11) NOT NULL default '0',
  exterminates int(11) NOT NULL default '0',
  spawns int(11) NOT NULL default '0',
  raids int(11) NOT NULL default '0',
  unique_logins int(11) NOT NULL default '0',
  accounts_paid int(11) NOT NULL default '0',
  accounts_expaid int(11) NOT NULL default '0',
  accounts_active int(11) NOT NULL default '0',
  accounts_total int(11) NOT NULL default '0',
  PCU int(11) NOT NULL default '0',
  tokens_bought int(11) NOT NULL default '0',
  tokens_used int(11) NOT NULL default '0',
  db_query_cnt int(11) NOT NULL default '0',
  db_query_time int(11) NOT NULL default '0',
  db_query_long int(11) NOT NULL default '0',
  PRIMARY KEY  (name)
);


CREATE TABLE clans (
  ID int(11) NOT NULL,
  data blob default null,
  PRIMARY KEY  (ID)
);



CREATE TABLE depot (
  ID int(11) NOT NULL default '0',
  area int(11) NOT NULL default '0',
  mirror int(11) NOT NULL default '0',
  cID int(11) NOT NULL default '0',
  data blob default null,
  PRIMARY KEY  (ID)
);


CREATE TABLE vars (
  name varchar(8) NOT NULL,
  val int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (name)
);

