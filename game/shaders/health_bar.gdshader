shader_type spatial;
render_mode unshaded;

const vec4 empty_color = vec4(0.0, 0.0, 0.0, 0.0);
const vec4 health_color = vec4(0.0, 1.0, 0.0, 1.0);
const vec4 armor_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 shield_color = vec4(0.25, 0.5, 1.0, 1.0);

uniform float health : hint_range(0.0, 1.0) = 1.0;
uniform float armor : hint_range(0.0, 1.0) = 0.0;
uniform float shield : hint_range(0.0, 1.0) = 0.0;

//uniform vec4 color_picker : source_color = vec4(0.0, 0.0, 0.0, 0.0);

void vertex() {
	MODELVIEW_MATRIX = VIEW_MATRIX * mat4(INV_VIEW_MATRIX[0], INV_VIEW_MATRIX[1], INV_VIEW_MATRIX[2], MODEL_MATRIX[3]);
}

void fragment() {
	ALBEDO = empty_color.rgb;
	ALPHA = empty_color.a;
	if(UV.x < shield) {
		ALBEDO = shield_color.rgb;
		ALPHA = shield_color.a;
	}
	if(UV.x < armor) {
		ALBEDO = armor_color.rgb;
		ALPHA = armor_color.a;
	}
	if(UV.x < health) {
		ALBEDO = health_color.rgb;
		ALPHA = health_color.a;
	}
}
