PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE Users(name TEXT NOT NULL, password TEXT NOT NULL, email TEXT NOT NULL);
INSERT INTO Users VALUES('Felpork','password','sneed123456@yahoo.com');
INSERT INTO Users VALUES('Skimat','12345678','whatisthisname@aol.com');
COMMIT;
