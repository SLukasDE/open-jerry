#sqlite3 my.db
sqlite> CREATE TABLE users(USER_ID TEXT NOT NULL, PASSWD TEXT NOT NULL, IS_ADMIN INT, GRANT_USERS INT);
sqlite> INSERT INTO users (USER_ID, PASSWD, IS_ADMIN, GRANT_USERS) VALUES ('Hans1', 'plain:Wurst', 0, 1);
sqlite> INSERT INTO users (USER_ID, PASSWD, IS_ADMIN, GRANT_USERS) VALUES ('Hans3', 'plain:Wurst3', 1, 1);
#sqlite3 my.db .dump
UPDATE users SET PASSWD = 'plain:wurst3' WHERE USER_ID='Hans3';
