--
-- Current Database: `Clic500TubeX01`
--
DROP DATABASE IF EXISTS `Clic500TubeX01_20`;
CREATE DATABASE  `Clic500TubeX01_20` ;
USE `Clic500TubeX01_20`;
--
-- Table structure for table `_references`
--
DROP TABLE IF EXISTS `_references`;
CREATE TABLE `_references` (
  `globalName` varchar(80) default NULL,
  `localName` varchar(32) default NULL,
  `assumption` double default NULL
);
--
-- Dumping data for table `_references`
--
-- LOCK TABLES `_references` WRITE; INSERT INTO `_references` VALUES
-- ('Ecal_endcap_zmax','ecalEnd',2500),
-- ('VXD_inner_radius','vtxInnerR',31),
-- ('LHcal_inner_radius','lhcalInnerR',132),
-- ('Lcal_outer_radius','lcalOuterR',195.2),
-- ('Lcal_inner_radius','lcalInnerR',85),
-- ('LHcal_zend','lhcalEnd', 2600),
-- ('Lcal_z_begin','lcalBegin',2342.4); -- this will change?? What does it do??? -- Do I even need references????
-- UNLOCK TABLES;
--
-- Table structure for table `tube`
--
DROP TABLE IF EXISTS `tube`;
CREATE TABLE `tube` (
  `crossType` int(11) default NULL,
  `zStart` double default NULL,
  `zEnd` double default NULL,
  `rInnerStart` double default NULL,
  `rInnerEnd` double default NULL,
  `rOuterStart` double default NULL,
  `rOuterEnd` double default NULL,
  `material` varchar(80) default NULL,
  `name` varchar(32) default NULL,
  `zStartRef` varchar(32) default NULL,
  `zEndRef` varchar(32) default NULL,
  `rInnerStartRef` varchar(32) default NULL,
  `rInnerEndRef` varchar(32) default NULL,
  `rOuterStartRef` varchar(32) default NULL,
  `rOuterEndRef` varchar(32) default NULL
);
--
-- Dumping data for table `tube`
--
LOCK TABLES `tube` WRITE; INSERT INTO `tube` VALUES	
(0,	0,	135,	23.5,	23.5,	24,	24,	'beryllium',	'IPInnerTube',	'',	'',	'',	'',	'',	''),	
(0,	135,	208,	23.5,	23.5,	24.696,	24.696,	'beryllium',	'IPOuterTube',	'',	'',	'',	'',	'',	''),	
(0,	208,	238.33,	23.5,	23.5,	24.,	27.5,	'iron',	'CylinderConeConnect',	'',	'',	'',	'',	'',	''),	
-- THIS NAME MUST STAY!!! Is Connected to FTDs!!!	
(0,	238.33,	2080,	23.5,	235.2,	27.5,	240,	'iron',	'IPOuterBulge',	'',	'',	'',	'',	'',	''),
(0,	2080,	2645,	235.2,	235.2,	240,	240,	'iron',	'IPOuterLink',	'',	'',	'',	'',	'',	''),	
(3,	2645,	2646,	0,	98,	240,	240,	'iron',	'LumiCalFront',	'',	'',	'',	'',	'',	''),	
(7,	2646,	3170,	98,	98,	99,	99,	'iron',	'LumiCalInner',	'',	'',	'',	'',	'',	''),
(5,	3170,	3171,	2.7,	31,	99,	99,	'iron',	'BeamCalFront',	'',	'',	'',	'',	'',     ''),	
(2,	3171,	3500,	31,	31,	32,	32,	'iron',	'BeamCalInnerDnstream1',	'',	'',	'',	'',	'',	''),	
(2,	3500, 	4000,	31,	39,	32,	40,	'iron',	'BehindBeamCalDnstream2',	'',	'',	'',	'',	'',	''),
(2,	4000, 	7000,	39,	69,	40,	70,	'iron',	'BehindBeamCalDnstream3',	'',	'',	'',	'',	'',	''),
(2,	7000, 	12500,	69,	124,	70,	125,	'iron',	'BeamCalInnerDnstream4',	'',	'',	'',	'',	'',	''),
(12,	3171,	3500,	2.7,	2.7,	3.7,	3.7,	'iron',	'BeamCalInnerUpstream',	'',	'',	'',	'',	'',	''),	
(1,	3500,	4000,	2.7,	2.7,	3.7,	3.7,	'iron',	'MainUpstream1',	'',	'',	'',	'',	'',	''),
(1,	4000,	6000,	2.7,	2.7,	3.7,	3.7,	'iron',	'MainUpstream2',	'',	'',	'',	'',	'',	''),
(1,	6000,	8000,	2.7,	2.7,	3.7,	3.7,	'iron',	'MainUpstream3',	'',	'',	'',	'',	'',	''),
(1,	8000,	10000,	2.7,	2.7,	3.7,	3.7,	'iron',	'MainUpstream4',	'',	'',	'',	'',	'',	''),
(1,	10000,	12500,	2.7,	2.7,	3.7,	3.7,	'iron',	'MainUpstream5',	'',	'',	'',	'',	'',	'');
UNLOCK TABLES;

use 'models03';

DELETE FROM `sub_detector` WHERE name='Clic500TubeX01';
INSERT INTO `sub_detector` 
       (name, db, driver, description, subdriver) VALUES 
       ('Clic500TubeX01','Clic500TubeX01','tubeX01','Tube for 500GeV CLIC_ILD_CDR', '');