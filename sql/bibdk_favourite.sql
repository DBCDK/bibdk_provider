drop table bibdk_favourite;
create table bibdk_favourite( favourite_id serial, username character varying(64), agencyid character varying(16));
alter table bibdk_favourite add constraint PK_favourite_id primary key(favourite_id);
alter table bibdk_favourite add constraint FK_username foreign key(username) references ddbuser(username) on delete cascade;
create index bibdk_favourite_agencyid_idx on bibdk_favourite (agencyid);
comment on column bibdk_favourite.username is 'bibdk username is email address. References ddbuser.username. cascade on delete';
comment on column bibdk_favourite.agencyid is 'id of favourite agency';