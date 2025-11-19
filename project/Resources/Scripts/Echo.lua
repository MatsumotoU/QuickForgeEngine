function Init()

end

function Update()
    if QFE.Input.GetKeyTrigger("Jump") then
        Echo(transform.translate,3.0)
    end
end
