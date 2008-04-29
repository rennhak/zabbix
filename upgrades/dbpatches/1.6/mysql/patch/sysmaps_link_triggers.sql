-- TODO Populate data from sysmaps_links

CREATE TABLE sysmaps_link_triggers (
        linktriggerid           bigint unsigned         DEFAULT '0'     NOT NULL,
        linkid          bigint unsigned         DEFAULT '0'     NOT NULL,
        triggerid               bigint unsigned         DEFAULT '0'     NOT NULL,
        drawtype                integer         DEFAULT '0'     NOT NULL,
        color           varchar(6)              DEFAULT '000000'        NOT NULL,
        PRIMARY KEY (linktriggerid)
) type=InnoDB;
CREATE UNIQUE INDEX sysmaps_link_triggers_1 on sysmaps_link_triggers (linkid,triggerid);
