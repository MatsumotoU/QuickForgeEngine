function Init()
	
end

function Update()
	this.transform.scale.y = math.sin(this.timeCounter*2.0) * 0.1 + 0.9
	this.transform.scale.x = math.cos(this.timeCounter*2.0) * 0.3 + 0.7
	this.transform.scale.z = math.cos(this.timeCounter*2.0) * 0.3 + 0.7

	this.transform.rotate.y = this.transform.rotate.y + 0.01
end

function Collision()
	
end

