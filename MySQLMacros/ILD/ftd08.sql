-- MySQL dump 10.13  Distrib 5.1.49, for debian-linux-gnu (i486)
--
-- Host: localhost    Database: ftd08
-- ------------------------------------------------------
-- Server version	5.1.49-3

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `ftd08`
--

DROP DATABASE if exists `ftd08`;

CREATE DATABASE  `ftd08`;

USE `ftd08`;

--
-- Table structure for table `common_parameters`
--

DROP TABLE IF EXISTS `common_parameters`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `common_parameters` (
  `ftd1_vtx3_distance_z` double DEFAULT NULL,
  `ftd7_ecal_distance_z` double DEFAULT NULL,
  `ftd1_sit1_radial_diff` double DEFAULT NULL,
  `ftd2_sit1_radial_diff` double DEFAULT NULL,
  `ftd3_sit2_radial_diff` double DEFAULT NULL,
  `ftd4to7_tpc_radial_gap` double DEFAULT NULL,
  `beamtube_clearance` double DEFAULT NULL,
  `cables_thickness` double DEFAULT NULL,
  `cable_shield_thickness` double DEFAULT NULL,
  `outer_cylinder_total_thickness` double DEFAULT NULL,
  `petal_half_angle_support` double DEFAULT '11.25',
  `petal_wings_length` double DEFAULT '5',
  `petal_wings_thickness` double DEFAULT '0.8',
  `petal_y_ratio` double DEFAULT '0.5015',
  `chip_strips_thickness` double DEFAULT '0.05',
  `chip_strips_width` double DEFAULT '10',
  `chip_strips_length` double DEFAULT '12'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `common_parameters`
--

LOCK TABLES `common_parameters` WRITE;
/*!40000 ALTER TABLE `common_parameters` DISABLE KEYS */;
INSERT INTO `common_parameters` VALUES (95,200,-1,-1,-1,20,15,0.08,0.1,1,11.25,5,0.8,0.5015,0.05,10,12);
/*!40000 ALTER TABLE `common_parameters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `disks`
--

DROP TABLE IF EXISTS `disks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `disks` (
  `disk_number` int(11) DEFAULT NULL,
  `z_position_ReltoTPCLength` double DEFAULT NULL,
  `disk_si_thickness` double DEFAULT NULL,
  `petal_cp_support_dxMax` double DEFAULT '122.49',
  `petal_cp_holes_separation` double DEFAULT NULL,
  `petal_cp_holes_edges_separation` double DEFAULT NULL,
  `petal_cp_holes_width_support` double DEFAULT '6',
  `padUp_Si_dxMax` double DEFAULT '118.46',
  `petal_cp_support_thickness` double DEFAULT '2',
  `petal_inclination_angle_support` double DEFAULT '4',
  `kapton_petal_thickness` double DEFAULT '0.1',
  `kapton_petal_interspace` double DEFAULT '2',
  `petal_support_zoffset` double DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `disks`
--

LOCK TABLES `disks` WRITE;
/*!40000 ALTER TABLE `disks` DISABLE KEYS */;
INSERT INTO `disks` VALUES (1,0,0.02,76.67,10,5,6,118.46,1,6,0.1,2,0),(2,0.158004,0.02,76.67,10,5,6,118.46,1,6,0.1,2,0),(3,0.274428,0.025,122.49,10,5,6,118.46,2,4,0.1,2,0),(4,0.445156,0.275,122.49,10,5,6,118.46,2,4,0.1,2,0),(5,0.615884,0.275,122.49,10,5,6,118.46,2,4,0.1,2,0),(6,0.786611,0.275,122.49,10,5,6,118.46,2,4,0.1,2,0),(7,0,0.275,122.49,10,5,6,118.46,2,4,0.1,2,0);
/*!40000 ALTER TABLE `disks` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-08-03 15:05:13
