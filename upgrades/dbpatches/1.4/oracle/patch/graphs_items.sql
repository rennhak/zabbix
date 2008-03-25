CREATE TABLE graphs_items_tmp (
        gitemid         number(20)              DEFAULT '0'     NOT NULL,
        graphid         number(20)              DEFAULT '0'     NOT NULL,
        itemid          number(20)              DEFAULT '0'     NOT NULL,
        drawtype                number(10)              DEFAULT '0'     NOT NULL,
        sortorder               number(10)              DEFAULT '0'     NOT NULL,
        color           varchar2(32)            DEFAULT 'Dark Green'    ,
        yaxisside               number(10)              DEFAULT '1'     NOT NULL,
        calc_fnc                number(10)              DEFAULT '2'     NOT NULL,
        type            number(10)              DEFAULT '0'     NOT NULL,
        periods_cnt             number(10)              DEFAULT '5'     NOT NULL,
        PRIMARY KEY (gitemid)
);

insert into graphs_items_tmp select * from graphs_items;
drop table graphs_items;
alter table graphs_items_tmp rename graphs_items;