import bpy
import bpy_extras
import math
import gpu
import gpu_extras.batch
import copy
import mathutils

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

class DrawCollider:
    handle = None

    offsets = [
        [0.5,0.5,0.5],
        [0.5,0.5,-0.5],
        [0.5,-0.5,0.5],
        [0.5,-0.5,-0.5],
        [-0.5,0.5,0.5],
        [-0.5,0.5,-0.5],
        [-0.5,-0.5,0.5],
        [-0.5,-0.5,-0.5]
        ]

    size = [2.0,2.0,2.0]

    @staticmethod
    def draw_collider():
        vertices = {"pos":[]}
        indices = []

        for object in bpy.context.scene.objects:
            if not "collider" in object:
                continue

            center = mathutils.Vector((0.0,0.0,0.0))
            size = mathutils.Vector((2.0,2.0,2.0))
            center[0] = object["collider_center"][0]
            center[1] = object["collider_center"][1]
            center[2] = object["collider_center"][2]
            size[0] = object["collider_size"][0]
            size[1] = object["collider_size"][1]
            size[2] = object["collider_size"][2]

            start = len(vertices["pos"])
            for offset in DrawCollider.offsets:
                pos = copy.copy(center)
                pos[0] += offset[0] * size[0]
                pos[1] += offset[1] * size[1]
                pos[2] += offset[2] * size[2]
                pos = object.matrix_world @ pos
                vertices["pos"].append(pos)

                indices.append([start+0,start+1])
                indices.append([start+2,start+3])
                indices.append([start+0,start+2])
                indices.append([start+1,start+3])

                indices.append([start+4,start+5])
                indices.append([start+6,start+7])
                indices.append([start+4,start+6])
                indices.append([start+5,start+7])

                indices.append([start+0,start+4])
                indices.append([start+1,start+5])
                indices.append([start+2,start+6])
                indices.append([start+3,start+7])

        shader = gpu.shader.from_builtin('UNIFORM_COLOR')
        batch = gpu_extras.batch.batch_for_shader(shader,'LINES',vertices,indices=indices)
        color = [0.5,1.0,1.0,1.0]
        shader.bind()
        shader.uniform_float("color",color)
        batch.draw(shader)

class OBJECT_PT_collider(bpy.types.Panel):
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        if "collider" in context.object:
            self.layout.prop(context.object, '["collider"]', text=self.bl_label)
            self.layout.prop(context.object, '["collider_center"]', text="中心座標")
            self.layout.prop(context.object, '["collider_size"]', text="サイズ")
        else:
            self.layout.operator(MYADDON_OT_add_collider.bl_idname)

class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "object.myaddon_ot_add_collider"
    bl_label = "コライダーを追加します"
    bl_description = "コライダーを追加します"
    bl_options = {'REGISTER','UNDO'}
    def execute(self, context):
        context.object["collider"] = "BOX"
        context.object["collider_center"] = mathutils.Vector((0.0,0.0,0.0))
        context.object["collider_size"] = mathutils.Vector((2.0,2.0,2.0))
        return {'FINISHED'}

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
        
        self.layout.label(text = "Hello")
        self.layout.separator()
        self.layout.label(text="Hello2",icon="MESH_CUBE")

        self.layout.operator(MYDDON_OT_stretch_vertex.bl_idname,text=MYDDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYDDON_OT_create_ico_sphere.bl_idname,text=MYDDON_OT_create_ico_sphere.bl_label)
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
        
        if "collider" in object:
            self.write_and_print(file,indent + "Collider(%s)" % object["collider"])
            temp_str = indent + "CC %f,%f,%f" % (object["collider_center"][0],object["collider_center"][1],object["collider_center"][2])
            self.write_and_print(file,temp_str)
            temp_str = indent + "CS %f,%f,%f" % (object["collider_size"][0],object["collider_size"][1],object["collider_size"][2])
            self.write_and_print(file,temp_str)

        if "file_name" in object:
            self.write_and_print(file,indent + "FileName(%s)" % object["file_name"])
        self.write_and_print(file,indent+'END')
        self.write_and_print(file,'')
        
        self.write_and_print(file,'')
        for child in object.children:
            self.parse_scene_recursice(file,child,level+1)

    def export(self):
        """ ファイルに出力 """
        print("シーン情報出力開始... %r" % self.filepath)
        with open(self.filepath, "w", encoding="utf-8") as file:
            for object in bpy.context.scene.objects:
                if object.parent is None:
                    self.parse_scene_recursice(file, object, 0)

    def execute(self,context):
        print("シーンをエクスポートします")

        self.export()
        print("シーンをエクスポートしました")
        self.report({'INFO'}, "シーンをエクスポートしました")
        return {'FINISHED'}

class MYDDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "object.myddon_ot_create_ico_sphere"
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
    MYADDON_OT_add_filename,
    MYADDON_OT_add_collider,
    OBJECT_PT_collider
}

def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_collider,(), 'WINDOW', 'POST_VIEW')

    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    print("enable LevelEditor")
    
def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)

    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle,'WINDOW')
    
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("unenable LevelEditor")