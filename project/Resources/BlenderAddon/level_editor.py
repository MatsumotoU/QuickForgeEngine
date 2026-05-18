import bpy
import bpy_extras
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

class OBJECT_PT_file_name(bpy.types.Panel):
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "ファイル名"
    bl_region_type = "PROPERTIES"
    bl_context = "object"

    def draw(self, context):
        if "file_name" in context.object:
            self.layout.prop(context.object, '["file_name"]', text=self.bl_label)
        else:
            self.layout.operator(MYADDON_OT_add_filename.bl_idname)
        
        self.layout.label(texxt = "Hello")
        self.layout.separator()
        self.layout.label(text="Hello2",icon="MESH_CUBE")

        self.layout.operator(MYDOON_OT_stretch_vertex.bl_idname,text=MYDOON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYDOON_OT_create_ico_sphere.bl_idname,text=MYDOON_OT_create_ico_sphere.bl_label)
        self.layout.operator(MYDDON_OT_export_scene.bl_idname,text=MYDDON_OT_export_scene.bl_label)

class MYADDON_OT_add_filename(bpy.types.Operator):
    bl_idname = "object.myaddon_ot_add_filename"
    bl_label = "ファイル名を追加します"
    bl_description = "ファイル名を追加します"
    bl_options = {'REGISTER','UNDO'}

    def execute(self, context):
        context.object["file_name"] = ""
        return {'FINISHED'}

class MYDDON_OT_export_scene(bpy.types.Operator):
    bl_idname = "object.myddon_ot_export_scene"
    bl_label = "シーンをエクスポートします"
    bl_description = "シーンをエクスポートします"

    filename_ext = ".scene"

    def write_and_print(self, file, text):
        print(text)

        file.write(text)
        file.write("\n")

    def parse_scene_recursice(self,file,object,level):
        indent = ''
        for i in range(level):
            indent += "\t"

        self.write_and_print(file,indent + object.type)
        trans,rot,scale = object.matrix_local.decompose()
        rot = rot.to_euler()
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)
        self.write_and_print(file,indent + "Trans(%f,%f,%f)" % (trans.x, trans.y, trans.z))
        self.write_and_print(file,indent + "Rot(%f,%f,%f)" % (rot.x, rot.y, rot.z))
        self.write_and_print(file,indent + "Scale(%f,%f,%f)" % (scale.x, scale.y, scale.z))
        
        if "file_name" in object:
            self.write_and_print(file,indent + "FileName(%s)" % object["file_name"])
        self.write_and_print(file,indent+'END')
        self.write_and_print(file,'')
        
        self.write_and_print(file,'')
        for child in object.children:
            self.parse_scene_recursice(file,child,level+1)

    def export(self)
        """ ファイルに出力 """
        print("シーン情報出力開始... %r" % self.filepath)
        for object in bpy.context.scene.objects:
            

        

    def execute(self,context):
        print("シーンをエクスポートします")

        self.export()
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
    OBJECT_PT_file_name,
    MYADDON_OT_add_filename
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