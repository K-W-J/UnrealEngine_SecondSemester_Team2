import unreal

def dump(label, obj):
    unreal.log_warning("=== " + label + " ===")
    for name in dir(obj):
        lower = name.lower()
        if any(key in lower for key in ("widget", "tree", "root", "construct", "slot", "canvas")):
            unreal.log_warning(name)

for cls_name in ("WidgetBlueprintFactory", "WidgetBlueprint", "WidgetTree", "CanvasPanel", "CanvasPanelSlot"):
    cls = getattr(unreal, cls_name, None)
    if cls:
        dump(cls_name, cls)
    else:
        unreal.log_warning("MISSING " + cls_name)