function Init()

end

function Update()

end

function OnCollisionEnter(id,obj)
    if obj.tag == "player" then
        LoadScene("TitleScene.json")
    end
    
end