create table testcases (id INTEGER PRIMARY KEY, masterfile VARCHAR(255));

create table builds (revision INTEGER PRIMARY KEY, timestamp VARCHAR(16));

create table runs (testcaseid INTEGER, revision INTEGER, status VARCHAR(32), renderedfile VARCHAR(255));
