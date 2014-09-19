DROP DATABASE IF EXISTS  `CLICFieldMap4TeslaNoQuad_20`;
CREATE DATABASE `CLICFieldMap4TeslaNoQuad_20`;
USE `CLICFieldMap4TeslaNoQuad_20`;
DROP TABLE IF EXISTS `_references`;
CREATE TABLE `_references` (
  `globalName` varchar(80) default NULL,
  `localName` varchar(32) default NULL,
  `assumption` double default NULL
);
LOCK TABLES `_references` WRITE;
UNLOCK TABLES;
DROP TABLE IF EXISTS `magnetic`;
CREATE TABLE `magnetic` (
  `fieldType` int(11) default NULL,
  `zStart` double default NULL,
  `zEnd` double default NULL,
  `rInner` double default NULL,
  `rOuter` double default NULL,
  `fieldValue` double default NULL,
  `fieldData` varchar(32) default NULL,
  `zStartRef` varchar(32) default NULL,
  `zEndRef` varchar(32) default NULL,
  `rInnerRef` varchar(32) default NULL,
  `rOuterRef` varchar(32) default NULL
);
LOCK TABLES `magnetic` WRITE;
INSERT INTO `magnetic` VALUES 
(5,0,6985,0,6985,1.09149486,'ILD_00_Brho_Bz','','','','');
UNLOCK TABLES;
USE `models03`;
DELETE FROM `sub_detector` WHERE name='FieldMap4TNoQuad';
INSERT INTO `sub_detector` 
       (name, db, driver, description, subdriver) VALUES 
       ('FieldMap4TNoQuad'  ,'CLICFieldMap4TeslaNoQuad',  'fieldX03','4 Tesla Field Map', '');
DELETE FROM `sharing` WHERE `driver`='FieldMap4TNoQuad'  ;
INSERT INTO `sharing` (driver, parameter, driver_default_value) VALUES	
       ('FieldMap4TNoQuad'  , 'ILC_Main_Crossing_Angle', NULL);
