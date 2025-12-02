function Init()

end

function Update()

end

function OnCollisionEnter(id,obj)
    Echo(transform.translate,1.0)
    destroy()
end