use 'models03';

DELETE FROM `ingredients` WHERE model='CLIC_ILD_CDR500';
INSERT INTO `ingredients` (model, sub_detector, build_order) VALUES
       ('CLIC_ILD_CDR500','SEtd02',250), -- unchanged
--       ('CLIC_ILD_CDR500','LHcal01',120), -- removed
       ('CLIC_ILD_CDR500','clictpc01',200), -- changed cathode							--- database
       ('CLIC_ILD_CDR500','ClicSFtd01',220), -- Change to CLIC FTD11, ClicSftd01					--- database
       ('CLIC_ILD_CDR500','SLcal02',89), -- unchanged except global parameter
       ('CLIC_ILD_CDR500','SEcal03',90), -- unchanged 
       ('CLIC_ILD_CDR500','SHcalSc02',110), -- changed global parameter
       ('CLIC_ILD_CDR500','SCoil02',400), -- unchanged global parameter
       ('CLIC_ILD_CDR500','clicyoke01',500), -- new								--- database
       ('CLIC_ILD_CDR500','Clic500TubeX01',150), -- Change to ClicTubeX01						--- database
       ('CLIC_ILD_CDR500','SSit03',210), -- unchanged, except global parameter
       ('CLIC_ILD_CDR500','SField01',1000), -- Use Old driver, which is now (being the next mokka release...) updated...
       ('CLIC_ILD_CDR500','Clic500Vxd01',20), -- clicvxd, new, update striplines					--- database
       ('CLIC_ILD_CDR500','SSet02',230), -- unchanged
       ('CLIC_ILD_CDR500','clicmaskX01',160), -- new, add pseudo vxd cables					--- database
       ('CLIC_ILD_CDR500','BeamCal08',650); -- unchanged

DELETE FROM `model` WHERE name='CLIC_ILD_CDR500';
INSERT INTO `model` VALUES
       ('CLIC_ILD_CDR500','CLIC_ILD_CDR500','CILD','frozen');

DELETE FROM `model_parameters` WHERE model='CLIC_ILD_CDR500';
INSERT INTO `model_parameters` VALUES 
-- For the Yoke
('CLIC_ILD_CDR500','Coil_extra_size','1906'),
('CLIC_ILD_CDR500','Coil_Yoke_radial_clearance','250'),
('CLIC_ILD_CDR500','Coil_Yoke_lateral_clearance','280'),
('CLIC_ILD_CDR500','Yoke_Z_start_endcaps', '4536'),
('CLIC_ILD_CDR500','Hcal_R_max', '3088.99'),

('CLIC_ILD_CDR500','Ecal_Barrel_halfZ','2350'),
('CLIC_ILD_CDR500','Ecal_endcap_extra_size','60.8'),
('CLIC_ILD_CDR500','Ecal_support_thickness','9.3'),
('CLIC_ILD_CDR500','Ecal_Tpc_gap','35'),

('CLIC_ILD_CDR500','Field_nominal_value','4.0'), 
('CLIC_ILD_CDR500','Yoke_BField_cutoff','1.5'), 
('CLIC_ILD_CDR500','FieldPropagator_LargestAcceptableStep',10),
('CLIC_ILD_CDR500','Hcal_back_plate_thickness','15'),
('CLIC_ILD_CDR500','Hcal_Coil_additional_gap','20.0'),
('CLIC_ILD_CDR500','Hcal_Ecal_gap','30'),

('CLIC_ILD_CDR500','Hcal_endcap_ecal_gap','15'),
('CLIC_ILD_CDR500','Hcal_endcap_sensitive_center_box','0.0'),
('CLIC_ILD_CDR500','Hcal_endcap_zmin','2670.7'),
('CLIC_ILD_CDR500','Hcal_nlayers','75'),
('CLIC_ILD_CDR500','Hcal_radiator_material','TungstenDens24'),
('CLIC_ILD_CDR500','Hcal_radiator_thickness','10'),
('CLIC_ILD_CDR500','Hcal_endcap_nlayers','60'),
('CLIC_ILD_CDR500','Hcal_endcap_radiator_material','Iron'),
('CLIC_ILD_CDR500','Hcal_endcap_radiator_thickness','20'),
('CLIC_ILD_CDR500','Hcal_sensitive_model','scintillator'),
('CLIC_ILD_CDR500','ILC_Main_Crossing_Angle','20'),

('CLIC_ILD_CDR500','TPC_ECal_gap','25.0'),
('CLIC_ILD_CDR500','TPC_inner_radius','329'),
('CLIC_ILD_CDR500','TPC_outer_radius','1808'),
('CLIC_ILD_CDR500','TUBE_crossing_angle','0'),
('CLIC_ILD_CDR500','TUBE_opening_angle','0.07876'),
('CLIC_ILD_CDR500','VXD_active_silicon_thickness','0.05'),
('CLIC_ILD_CDR500','VXD_inner_radius','25'),
('CLIC_ILD_CDR500','VXD_length_r1','130'),
('CLIC_ILD_CDR500','VXD_length_r3','130'),
('CLIC_ILD_CDR500','VXD_length_r5','130'),
('CLIC_ILD_CDR500','VXD_radius_r1','25'),
('CLIC_ILD_CDR500','VXD_radius_r3','42'),
('CLIC_ILD_CDR500','VXD_radius_r5','58'),
('CLIC_ILD_CDR500','VXD_side_band_electronics_width','0.5'),
('CLIC_ILD_CDR500','VXD_support_ladder_material','\"graphite\"'),
('CLIC_ILD_CDR500','VXD_support_ladder_thickness','0.134'),
('CLIC_ILD_CDR500','Yoke_endcap_inner_radius','690'),
('CLIC_ILD_CDR500','Yoke_thickness','2550'),
('CLIC_ILD_CDR500','TPC_Ecal_Hcal_barrel_halfZ',2350),
('CLIC_ILD_CDR500','VXD_cryostat_option',0),
('CLIC_ILD_CDR500','VXD_end_ladd_electronics_option',1),
('CLIC_ILD_CDR500','SIT1_Radius', 165),
('CLIC_ILD_CDR500','SIT2_Radius', 309),
('CLIC_ILD_CDR500','VXD_end_ladd_electronics_half_length', 1),
('CLIC_ILD_CDR500','VXD_end_ladd_electronics_thickness', 0.000001),



-- For LumiCal

('CLIC_ILD_CDR500','Ecal_endcap_plug_rmin',290),-- the ecal inner radius is 302 mm
('CLIC_ILD_CDR500','Ecal_Lcal_ring_gap',-48),
-- ('CLIC_ILD_CDR500','Ecal_endcap_zmin',2450),
('CLIC_ILD_CDR500','Ecal_endcap_zmax',2825),
('CLIC_ILD_CDR500','Lcal_z_begin',2800),
('CLIC_ILD_CDR500','Lcal_inner_radius',100.0),
('CLIC_ILD_CDR500','Lcal_outer_radius',290),
('CLIC_ILD_CDR500','Hcal_endcap_center_box_size',800),
('CLIC_ILD_CDR500','Ecal_endcap_center_box_size',800),
('CLIC_ILD_CDR500','Lcal_n_layers',40),
('CLIC_ILD_CDR500','Lcal_z_thickness',135.6),
-- For the BeamCal Driver
('CLIC_ILD_CDR500'	,'LHcal_zend',2600),
('CLIC_ILD_CDR500','BCal_TubeIncomingRadius', 3.7),
('CLIC_ILD_CDR500','BCal_rInner'		 , 32), 
('CLIC_ILD_CDR500','BCal_rOuter'		 , 150),
('CLIC_ILD_CDR500','BCal_nLayers'		 , 40), 
('CLIC_ILD_CDR500','BCal_PairMonitor'	 , 0),  
('CLIC_ILD_CDR500','BCal_dAbsorber'	 , 3.5),
('CLIC_ILD_CDR500','BCal_dGraphite'	 , 100),
('CLIC_ILD_CDR500','BCal_rSegmentation'	 , 8),  
('CLIC_ILD_CDR500','BCal_nWafers'		 , 8),  
('CLIC_ILD_CDR500','BCal_SpanningPhi'	 , 350),
('CLIC_ILD_CDR500','LHcal_BCal_clearance'	 , 690);

