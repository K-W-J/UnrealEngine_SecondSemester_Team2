import unreal
for p in ['/Game/RadarSystem/Blueprints/BP_InvisibleRadar','/Game/RadarSystem/Blueprints/BP_Radar']:
    a=unreal.load_asset(p)
    unreal.log_warning('RADAR_EXPORT '+unreal.Exporter.export_to_file(a,unreal.Paths.project_saved_dir()+'/radar_'+a.get_name()+'.copy').__str__())
for p in ['/Game/RadarSystem/Materials/M_Radar','/Game/RadarSystem/Materials/MI_Radar']:
    a=unreal.load_asset(p)
    unreal.log_warning('RADAR_MAT '+p+' '+str(a))
    if isinstance(a,unreal.Material): unreal.log_warning('DOMAIN '+str(a.get_editor_property('material_domain')))
    for name in unreal.MaterialEditingLibrary.get_scalar_parameter_names(a): unreal.log_warning('SCALAR '+str(name))
