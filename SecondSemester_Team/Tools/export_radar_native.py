import unreal
for p in ['/Game/RadarSystem/Blueprints/BP_InvisibleRadar','/Game/RadarSystem/Blueprints/BP_Radar','/Game/RadarSystem/Materials/M_Radar']:
    a=unreal.load_asset(p)
    task=unreal.AssetExportTask()
    task.object=a
    task.filename=unreal.Paths.project_saved_dir()+'/inspect_'+a.get_name()+'.copy'
    task.automated=True
    task.prompt=False
    task.replace_identical=True
    unreal.log_warning('EXPORT '+str(unreal.Exporter.run_asset_export_task(task)))
    if isinstance(a,unreal.Blueprint):
        cdo=unreal.get_default_object(a.generated_class())
        unreal.log_warning('CDO '+str(cdo))
        unreal.log_warning('ATTR '+str([n for n in dir(cdo) if not n.startswith('_')]))
    else:
        unreal.log_warning('DOMAIN '+str(a.get_editor_property('material_domain')))
