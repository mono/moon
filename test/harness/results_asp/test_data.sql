
insert into testcases values (1,"test1_Master.jpg");
insert into testcases values (2,"test2_Master.jpg");
insert into testcases values (3,"test3_Master.jpg");
insert into testcases values (4,"test4_Master.jpg");
insert into testcases values (5,"test5_Master.jpg");
insert into testcases values (6,"test6_Master.jpg");

insert into builds values (1001, "2008-01-01");
insert into builds values (2001, "2008-02-01");
insert into builds values (3001, "2008-03-01");
insert into builds values (4001, "2008-04-01");

insert into runs values (1, 1001, "FAIL","test1_r1001.jpg");
insert into runs values (1, 2001, "FAIL","test1_r2001.jpg");
insert into runs values (1, 3001, "IGNORE","test1_r3001.jpg");
insert into runs values (1, 4001, "FAIL","test1_r4001.jpg");

insert into runs values (2, 1001, "PASS","test2_r1001.jpg");
insert into runs values (2, 3001, "PASS","test2_r3001.jpg");
insert into runs values (2, 4001, "PASS","test2_r4001.jpg");

insert into runs values (3, 1001, "IGNORE","test3_r1001.jpg");
insert into runs values (3, 2001, "FAIL","test3_r2001.jpg");
insert into runs values (3, 3001, "PASS","test3_r3001.jpg");

insert into runs values (4, 2001, "FAIL","test4_r2001.jpg");
insert into runs values (4, 3001, "FAIL","test4_r3001.jpg");
insert into runs values (4, 4001, "PASS","test4_r4001.jpg");
