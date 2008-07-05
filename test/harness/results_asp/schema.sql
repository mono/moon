CREATE TABLE builds (
	revision INTEGER,
	builddate DATE
);

CREATE TABLE testcases (
	id INTEGER PRIMARY KEY,
	desc VARCHAR(255),
	masterfile VARCHAR(255)
);

CREATE TABLE results (
	testcaseid INTEGER,
	runtime DATE,
	status VARCHAR(32),
	renderedfile VARCHAR(255)
);

CREATE TABLE taggedcases (
	testcaseid INTEGER,
	tagid INTEGER
);

CREATE TABLE tags (
	id INTEGER PRIMARY KEY,
	name VARCHAR(32)
);

