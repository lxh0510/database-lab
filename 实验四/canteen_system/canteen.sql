/*==============================================================*/
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2022/12/6 21:05:01                           */
/*==============================================================*/


/*==============================================================*/
/* Table: admininfo                                             */
/*==============================================================*/
create table admininfo
(
   adminID              int not null,
   adminName            varchar(10) not null,
   adminPassword        varchar(10) not null,
   adminPhone           numeric(11,0) not null,
   primary key (adminID)
);

/*==============================================================*/
/* Table: business                                              */
/*==============================================================*/
create table business
(
   businessID           int not null,
   canteenID            int,
   businessName         varchar(10) not null,
   businessPassword     varchar(10) not null,
   businessPhone        numeric(11,0) not null,
   Storename            varchar(10) not null,
   Minamount            int not null,
   Delivery             int not null,
   businessStatus       varchar(3) not null,
   primary key (businessID)
);

/*==============================================================*/
/* Table: canteen                                               */
/*==============================================================*/
create table canteen
(
   canteenID            int not null,
   canteenName          char(5) not null,
   openTime             time not null,
   endTime              time not null,
   primary key (canteenID)
);

/*==============================================================*/
/* Table: category                                              */
/*==============================================================*/
create table category
(
   categoryID           int not null,
   categoryName         varchar(10) not null,
   primary key (categoryID)
);

/*==============================================================*/
/* Table: meal                                                  */
/*==============================================================*/
create table meal
(
   mealID               int not null,
   businessID           int,
   categoryID           int,
   mealName             varchar(10) not null,
   Description          text not null,
   mealLeft             int not null,
   Graph                longblob not null,
   Price                int,
   primary key (mealID)
);

/*==============================================================*/
/* Index: idx_meal_name                                         */
/*==============================================================*/
create index idx_meal_name on meal
(
   mealName
);

/*==============================================================*/
/* Table: news                                                  */
/*==============================================================*/
create table news
(
   newsID               int not null,
   adminID              int,
   businessID           int,
   newsContent          text not null,
   isExamine            bool not null,
   primary key (newsID)
);

/*==============================================================*/
/* Table: order_meal                                            */
/*==============================================================*/
create table order_meal
(
   mealID               int not null,
   orderID              int not null,
   mealNumber           int not null,
   primary key (mealID, orderID)
);

/*==============================================================*/
/* Table: orderinfo                                             */
/*==============================================================*/
create table orderinfo
(
   orderID              int not null,
   orderName            varchar(10) not null,
   Storename            varchar(10) not null,
   canteenName          char(5) not null,
   newName              varchar(10),
   totalPrice           float(5) not null,
   Address              text not null,
   Time                 time not null,
   orderStatus          varchar(3) not null,
   primary key (orderID)
);

/*==============================================================*/
/* Table: put_orders                                            */
/*==============================================================*/
create table put_orders
(
   userID               int not null,
   orderID              int not null,
   orderTime            time not null,
   orderNumber          int not null,
   primary key (userID, orderID)
);

/*==============================================================*/
/* Table: receive_orders                                        */
/*==============================================================*/
create table receive_orders
(
   businessID           int not null,
   orderID              int not null,
   receiveTime          time not null,
   primary key (businessID, orderID)
);

/*==============================================================*/
/* Table: userinfo                                              */
/*==============================================================*/
create table userinfo
(
   userID               int not null,
   sex                  varchar(1),
   userPassword         varchar(10) not null,
   userPhone            numeric(11,0),
   userName             varchar(10) not null,
   primary key (userID)
);

/*==============================================================*/
/* View: viewMealInfo                                           */
/*==============================================================*/
create  VIEW    viewMealInfo
 as
select meal.mealID,meal.mealName,meal.Description,meal.mealLeft,meal.Graph,meal.Price,business.Storename,category.categoryName
from meal,business,category
where meal.businessID = business.businessID
and meal.categoryID = category.categoryID;

alter table business add constraint FK_business_canteen foreign key (canteenID)
      references canteen (canteenID) on delete restrict on update restrict;

alter table meal add constraint FK_business_meal foreign key (businessID)
      references business (businessID) on delete restrict on update restrict;

alter table meal add constraint FK_meal_category foreign key (categoryID)
      references category (categoryID) on delete restrict on update restrict;

alter table news add constraint FK_admin_news foreign key (adminID)
      references admininfo (adminID) on delete restrict on update restrict;

alter table news add constraint FK_business_news foreign key (businessID)
      references business (businessID) on delete restrict on update restrict;

alter table order_meal add constraint FK_order_meal foreign key (orderID)
      references orderinfo (orderID) on delete restrict on update restrict;

alter table order_meal add constraint FK_order_meal2 foreign key (mealID)
      references meal (mealID) on delete restrict on update restrict;

alter table put_orders add constraint FK_put_orders foreign key (orderID)
      references orderinfo (orderID) on delete restrict on update restrict;

alter table put_orders add constraint FK_put_orders2 foreign key (userID)
      references userinfo (userID) on delete restrict on update restrict;

alter table receive_orders add constraint FK_receive_orders foreign key (orderID)
      references orderinfo (orderID) on delete restrict on update restrict;

alter table receive_orders add constraint FK_receive_orders2 foreign key (businessID)
      references business (businessID) on delete restrict on update restrict;


CREATE TRIGGER business_BEFORE_DELETE
BEFORE DELETE ON business
FOR EACH ROW
BEGIN
delete from meal
where  businessID = old.businessID
END;

