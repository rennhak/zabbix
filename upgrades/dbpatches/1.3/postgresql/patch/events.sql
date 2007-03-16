CREATE TABLE events (
	eventid		bigint	DEFAULT '0'	NOT NULL,
	source		integer	DEFAULT '0'	NOT NULL,
	sourceid	bigint	DEFAULT '0'	NOT NULL,
	clock		integer	DEFAULT '0'	NOT NULL,
	value		integer	DEFAULT '0'	NOT NULL,
	acknowledged	integer		DEFAULT '0'	NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 on events (triggerid,source,clock);
CREATE INDEX events_2 on events (clock);

insert into events select eventid,0,triggerid,clock,value,acknowledged from alarms;
drop table alarms;
