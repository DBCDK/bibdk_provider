drop table bibdk_favourite;
create table bibdk_favourite( favourite_id serial, username character varying(64) not null, agencyid character varying(16) not null, orderagency boolean not null default FALSE);
alter table bibdk_favourite add constraint PK_favourite_id primary key(favourite_id);
alter table bibdk_favourite add constraint FK_username foreign key(username) references ddbuser(username) on delete cascade;
create index bibdk_favourite_agencyid_idx on bibdk_favourite (agencyid);
comment on column bibdk_favourite.username is 'bibdk username is email address. References ddbuser.username. cascade on delete';
comment on column bibdk_favourite.agencyid is 'id of favourite agency';
comment on column bibdk_favourite.orderagency is 'selected agency to order from';