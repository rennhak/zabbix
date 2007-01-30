CREATE TABLE items_applications_tmp (
	itemappid		bigint unsigned		NOT NULL auto_increment,
	applicationid		bigint unsigned		DEFAULT '0'	NOT NULL,
	itemid		bigint unsigned		DEFAULT '0'	NOT NULL,
	PRIMARY KEY (itemappid)
);
CREATE INDEX items_applications_1 on items_applications_tmp (applicationid,itemid);

insert into items_applications_tmp select NULL,applicationid,applicationid from items_applications;
drop table items_applications;
alter table items_applications_tmp rename items_applications;

CREATE TABLE items_applications_tmp (
	itemappid		bigint unsigned		DEFAULT '0'	NOT NULL,
	applicationid		bigint unsigned		DEFAULT '0'	NOT NULL,
	itemid		bigint unsigned		DEFAULT '0'	NOT NULL,
	PRIMARY KEY (itemappid)
);
CREATE INDEX items_applications_1 on items_applications_tmp (applicationid,itemid);

insert into items_applications_tmp select * from items_applications;
drop table items_applications;
alter table items_applications_tmp rename items_applications;
