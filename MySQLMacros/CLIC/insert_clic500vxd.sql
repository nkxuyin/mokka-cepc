use 'models03';

DELETE FROM `sub_detector` WHERE name='Clic500Vxd01';
INSERT INTO `sub_detector` 
       (name, db, driver, description, subdriver) VALUES 
       ('Clic500Vxd01','Clic500Vxd01','SVxd03','VertexDetector for 500Gev CLIC_ILD_CDR, no endelectronics', 'vxd03');
--       ('ClicVxd01NoThresh','ClicVxd01','SVxd03','VertexDetector for CLIC_ILD_CDR, no Threshold', 'vxd04');

--
-- Current Database: `Clic500Vxd01`
--

DROP DATABASE if exists `Clic500Vxd01`;

CREATE DATABASE  `Clic500Vxd01`;

USE `Clic500Vxd01`;

--
-- Table structure for table `cryostat`
--

DROP TABLE IF EXISTS `cryostat`;
CREATE TABLE `cryostat` (
  `id` tinyint(4) NOT NULL default '0',
  `alu_skin_inner_radious` float NOT NULL default '0',
  `alu_skin_tickness` float NOT NULL default '0',
  `foam_inner_radious` float NOT NULL default '0',
  `foam_tickness` float NOT NULL default '0',
  `foam_half_z` float NOT NULL default '0',
  `endplate_inner_radious` float NOT NULL default '0',
  `cryostat_option` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
);

--
-- Dumping data for table `cryostat`
--

LOCK TABLES `cryostat` WRITE;
INSERT INTO `cryostat` VALUES (1,100,0.5,90,10,160,30.7,0);
UNLOCK TABLES;

--
-- Table structure for table `layer`
--

DROP TABLE IF EXISTS `layer`;
CREATE TABLE `layer` (
  `id` tinyint(4) NOT NULL default '0',
  `layer_radius` float NOT NULL default '0',
  `ladder_length` float NOT NULL default '0',
  `ladder_width` float NOT NULL default '0',
  `nb_ladder` float NOT NULL default '0',
  `ladder_gap` float NOT NULL default '0',
  `strip_line_final_z` float NOT NULL default '0',
  `end_electronics_width` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
);

--
-- Dumping data for table `layer`
--

LOCK TABLES `layer` WRITE;
INSERT INTO `layer` VALUES 
(1,25,130,11.3,12,0,135,1),
(2,25,130,11.3,12,0,135,1),
(3,42,130,11.3,12,0,135,1),
(4,42,130,11.3,12,0,135,1),
(5,58,130,11.3,20,0,135,1),
(6,58,130,11.3,20,0,135,1);
UNLOCK TABLES;
-- 136
--
-- Table structure for table `layers_common_parameters`
--

DROP TABLE IF EXISTS `layers_common_parameters`;
CREATE TABLE `layers_common_parameters` (
  `id` tinyint(4) NOT NULL default '0',
  `support_structure_thickness` float NOT NULL default '0',
  `electronics_structure_thickness` float NOT NULL default '0',
  `active_silicon_thickness` float NOT NULL default '0',
  `strip_lines_thickness` float NOT NULL default '0',
  `support_structure_radial_thickness` float NOT NULL default '0',
  `end_electronics_half_z` float NOT NULL default '0',
  `strip_final_beampipe_radious` float NOT NULL default '0',
  `side_band_electronics_option` int(11) NOT NULL default '0',
  `ladder_support_material` varchar(80) NOT NULL default '0',
  `end_ladd_electronics_option` int(11) NOT NULL default '0',
  `side_band_electronics_width` float NOT NULL default '0',
  `side_band_electronics_thickness` float NOT NULL default '0',
  `active_side_band_electronics_option` int(11) NOT NULL default '0',
  `layer_gap` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
);

--
-- Dumping data for table `layers_common_parameters`
--

LOCK TABLES `layers_common_parameters` WRITE;
INSERT INTO `layers_common_parameters` VALUES (1,0.067,0.1,0.05,0.09438,0.49392,5,24.6,1,'graphite',1,0.5,0.05,0,2);
UNLOCK TABLES;

--
-- Table structure for table `support_shell`
--

 DROP TABLE IF EXISTS `support_shell`;
 CREATE TABLE `support_shell` (
   `id` tinyint(4) NOT NULL default '0',
   `inner_radious` float NOT NULL default '0',
   `half_z` float NOT NULL default '0',
   `thickess` float NOT NULL default '0',
   `endplate_inner_radious` float NOT NULL default '0',
   `endplate_inner_radius_L1` float NOT NULL default '0',
   `endplate_outer_radius_L1` float NOT NULL default '0',
   `offset_ladder_block` float NOT NULL default '0',
   `beryllium_ladder_block_length` float NOT NULL default '0',
   `beryllium_ladder_block_thickness` float NOT NULL default '0',
   PRIMARY KEY  (`id`)
 );
-- 
-- --
-- -- Dumping data for table `support_shell`
-- --
-- 
LOCK TABLES `support_shell` WRITE;
   INSERT INTO `support_shell` VALUES (0,80,134,0.49392,29.0,14.5,40,0.28224,1,0.25);
-- INSERT INTO `support_shell` VALUES (0,80,135,0.49392,30.3,14.5,40,0.28224,1,0.25);
-- INSERT INTO `support_shell` VALUES (0,80,145,0.49392,30.3,14.5,40,0.28224,5,0.25);
UNLOCK TABLES;

-- Moved the support shell to where it is by default for the inner most layers,
-- Moved the support shell to where it is by default for the inner most layers
-- plus endband electronics 130 plus 4 times the electronic length it has to be
