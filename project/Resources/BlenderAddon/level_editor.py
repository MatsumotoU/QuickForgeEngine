import bpy
import math

bl_info = {
    "name": "LevelEditor",
    "author": "Matsumoto",
    "version": (1,0),
    "blender": (3,3,1),
    "location": "",
    "description": "LevelEditor",
    "support": "TESTING",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

class MYDDON_OT_export_scene(bpy.types.Operator):
    bl_idname = "object.myddon_ot_export_scene"
    bl_label = "シーンをエクスポートします"
    bl_description = "シーンをエクスポートします"

    def execute(self,context):
        print("シーンをエクスポートします")
        for object in bpy.context.scene.objects:
            print(object.type + "_" + object.name)
            trans,rot,scale = object.matrix_world.decompose()
            rot = rot.to_euler()
            rot.x = math.degrees(rot.x)
            rot.y = math.degrees(rot.y)
            rot.z = math.degrees(rot.z)
            print("Trans(%f,%f,%f) Rot(%f,%f,%f) Scale(%f,%f,%f)" % (trans.x, trans.y, trans.z, rot.x, rot.y, rot.z, scale.x, scale.y, scale.z))
            if object.parent:
                print("Parent(%s)" % object.parent.name)
            print()
        print("シーンをエクスポートしました")
        self.report({'INFO'}, "シーンをエクスポートしました")
        return {'FINISHED'}

class MYDDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "object.myddon_ot_create_object"
    bl_label = "ICO球を生成します"
    bl_description = "頂点座標を引っ張って伸ばします"
    bl_options = {'REGISTER','UNDO'}

    def execute(self,context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました")
        return {'FINISHED'}

class MYDDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "object.myddon_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張って伸ばします"
    bl_options = {'REGISTER','UNDO'}

    def execute(self,context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0
        print("頂点を伸ばしました")
        return {'FINISHED'}

class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "拡張メニュー by " + bl_info["author"]

    def draw(self,context):
        self.layout.operator("wm.url_open_preset",
            text = "Manual",icon = 'HELP')
        
        self.layout.operator(MYDDON_OT_stretch_vertex.bl_idname,
            text = MYDDON_OT_stretch_vertex.bl_label)
        
        self.layout.operator(MYDDON_OT_create_ico_sphere.bl_idname,
            text = MYDDON_OT_create_ico_sphere.bl_label)
        
    def submenu(self,context):
        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)

classes = {
    MYDDON_OT_export_scene,
    MYDDON_OT_create_ico_sphere,
    MYDDON_OT_stretch_vertex,
    TOPBAR_MT_my_menu,
}

def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    print("enable LevelEditor")
    
def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
    
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("unenable LevelEditor")