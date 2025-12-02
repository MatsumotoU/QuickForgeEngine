echoCharge = 0.0

function Init()
    echoCharge = 100.0
end

function Update()
    if echoCharge > 100.0 then
        if QFE.Input.GetKeyTrigger("Jump") then
            Echo(transform.translate,2.0)
            echoCharge = 0.0
        end
    else
        echoCharge = echoCharge +1.0;
    end
end
