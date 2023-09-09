import * as THREE from 'three';
import { OrbitControls } from './orbitcontrols_fixed.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';
import { LineGeometry } from 'three/addons/lines/LineGeometry.js';
import { LineMaterial } from 'three/addons/lines/LineMaterial.js';
import { Line2 } from 'three/addons/lines/Line2.js';

let scene;
let name_to_obj = {};
let controls;
let star_group = new THREE.Group();
let planets_group = new THREE.Group();
const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 1.0, 10000 );
const renderer = new THREE.WebGLRenderer({antialias: true, alpha: true });
let labelRenderer = new CSS2DRenderer();
let container = document.getElementById('glContainer');
let raycaster;
const pointer = new THREE.Vector2(99, 99);
let intersection_obj;
let all_data;
let mode = "galaxy";
let last_activation_ts;
let gridHelper;

function click_home()
{
    mode = "galaxy";
    controls.reset();
    controls.enableZoom = true;
    planets_group.clear();
    document.getElementById("system_indicator").classList.remove("active")
    for (const star of star_group.children) {
        star.visible = true;
        star.children[0].visible = true;
    }
    gridHelper.visible = true;
}

function get_new_elem(type, content=null, classlist=null)
{
    let result = document.createElement(type);
    if(content != null)
        result.textContent = content;
    if(classlist != null)
        result.classList = classlist;
    return result;
}

function get_infobox_attrib_el(attrib_name, attrib_value)
{
    let row = get_new_elem("div");
    row.appendChild(get_new_elem("div", attrib_name, "infobox_attrib_name"));
    row.appendChild(get_new_elem("div", attrib_value, "infobox_attrib_value"));
    return row;
}


// starfield seems to actually truncate numbers and not round them
function get_fixed_trunc(number, digits)
{
    return Math.trunc(number*Math.pow(10, digits))/Math.pow(10, digits)
}


function set_infobox_star(star_name, star)
{
    let infobox_el = document.getElementById("infobox");
    infobox_el.textContent = "";

    let level_indicator = get_new_elem("div", null, "level_indicator");
    level_indicator.appendChild(get_new_elem("div", "LEVEL"));
    level_indicator.appendChild(get_new_elem("div", star["level"]));
    infobox_el.appendChild(level_indicator);

    
    infobox_el.appendChild(get_new_elem("div", star_name, "infobox_name"));    

    let attrib_list_el = get_new_elem("div");
    attrib_list_el.id = "infobox_list";
    attrib_list_el.appendChild(get_infobox_attrib_el("spectral_class", star["spectral_class"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("temperature", `${star["temperature"]}K`));
    attrib_list_el.appendChild(get_infobox_attrib_el("mass", `${get_fixed_trunc(star["mass"], 2)}SM`));
    attrib_list_el.appendChild(get_infobox_attrib_el("radius", star["radius"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("magnitude", get_fixed_trunc(star["magnitude"], 2)));
    attrib_list_el.appendChild(get_infobox_attrib_el("planets", star["planet_count"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("moons", star["moon_count"]));
    infobox_el.appendChild(attrib_list_el);

    let button = get_new_elem("button", "goto_system");
    button.addEventListener("click", function(){activate_system(star_name); });
    infobox_el.appendChild(button);
}

function highlight_obj(obj, with_label, from_type)
{
    if(mode != "galaxy" && from_type == "star")
        return;
    obj.material.opacity = 1.0;
    if(with_label)
    {
        let label_div = obj.children[0].element;
        label_div.classList.add("forcedhover");
    }
}
function unhighlight_obj(obj, with_label, from_type)
{
    if(mode != "galaxy" && from_type == "star")
        return;
    obj.material.opacity = 0.5;
    if(with_label)
    {
        let label_div = obj.children[0].element;
        label_div.classList.remove("forcedhover");
    }
}

function xy_zero_orbit_controls(orbit_controls, new_height)
{
    let cam = orbit_controls.object;
    cam.position.set(0, new_height, new_height);
    orbit_controls.target.set(0, orbit_controls.target.y, 0);
    orbit_controls.update();
}

// function get_visual_radius(real_radius){
//     return 5.0;
// }

function add_system_body(color, radius, pos, receives_shadow)
{
    const geometry = new THREE.SphereGeometry( radius, 24, 8 );
    let material;
    if(receives_shadow == false)
        material = new THREE.MeshBasicMaterial( { color: color } ); 
    else
        material = new THREE.MeshStandardMaterial( { color: color } ); 
    const sphere = new THREE.Mesh( geometry, material );
    sphere.receiveShadow = receives_shadow;
    planets_group.add( sphere );
    sphere.position.set(pos.x, pos.y, pos.z);
}


function add_planet_orbit(center_vec3, radius, target_group)
{
    let line_points = [];
    const n = 64;
    for (let i = 0; i < n; i++)
    {
        const angle = 1.0*i/(n-1)*2.0*Math.PI;
        line_points.push(
            center_vec3.x + radius*Math.cos(angle),
            center_vec3.y,
            center_vec3.z + radius*Math.sin(angle)
            );
    }

    let line_geometry = new LineGeometry();
    line_geometry.setPositions( line_points );
    let line_material = new LineMaterial({color: 0xffffff, linewidth: 0.0005});
    line_material.transparent = true;
    line_material.opacity = 0.4;
    let rings_obj = new Line2( line_geometry, line_material);
    target_group.add(rings_obj)
}


function activate_system(star_name)
{
    mode = "system";
    document.getElementById("infobox").classList.remove("active");
    document.getElementById("system_indicator").classList.add("active")
    document.getElementById("system_indicator").textContent = star_name;
    for (const star of star_group.children) {
        star.visible = false;
        star.children[0].visible = false;
    }
    gridHelper.visible = false;
    controls.saveState();
    controls.enableZoom = true;
    xy_zero_orbit_controls(controls, 100.0);

    planets_group.clear();
    let planet_index = 0;
    for (const [key, planet] of Object.entries(all_data["stars"][star_name]["planets"]))
    {
        let dist_from_sun = 10.0 * (planet_index + 1);
        let planet_pos = new THREE.Vector3(dist_from_sun * Math.cos(planet["start_angle"]), 0.0, dist_from_sun * Math.sin(planet["start_angle"]) );
        add_system_body(0x407945, 4, planet_pos, true);
        add_planet_orbit(new THREE.Vector3(), dist_from_sun, planets_group);
        ++planet_index;
        console.log("planet_pos: " + planet_pos.toArray());

        for (const [moon_key, moon] of Object.entries(planet["moons"]))
        {
            let distance_from_planet = 6.0;
            let moon_pos = new THREE.Vector3();
            moon_pos.copy(planet_pos); // JS is absolutely insane
            moon_pos.add(new THREE.Vector3(distance_from_planet * Math.cos(moon["start_angle"]), 0.0, distance_from_planet * Math.sin(moon["start_angle"]) ) );
            add_system_body(0x808080, 1.5, moon_pos, true);
        }
    }
    add_system_body(0xffff80, 4, new THREE.Vector3(), false);
    scene.add( planets_group );
}

function on_label_click(name)
{
    set_infobox_star(name, all_data["stars"][name]);
    document.getElementById("infobox").classList.add("active");
    last_activation_ts = Date.now();

    // activate_system(name);
}
function on_label_mouseover(event, name)
{
    highlight_obj(name_to_obj[name], false, event.target.dataset.type);
}
function on_label_mouseout(event, name)
{
    unhighlight_obj(name_to_obj[name], false, event.target.dataset.type);
}

function on_container_click()
{
    if(intersection_obj !== null)
    {
        set_infobox_star(intersection_obj.name, all_data["stars"][intersection_obj.name]);
        last_activation_ts = Date.now();
        document.getElementById("infobox").classList.add("active");
    }
    else
    {
        if((Date.now() - last_activation_ts) > 200)
            document.getElementById("infobox").classList.remove("active");

    }
}



function add_galaxy_view_star(scene, position, name)
{
    const map = new THREE.TextureLoader().load( 'circle.png' );
    const material = new THREE.SpriteMaterial( { map: map, color: 0xff807d  } );
    material.transparent = true;
    material.opacity = 0.5;
    material.sizeAttenuation = false;
    const sprite = new THREE.Sprite( material );
    sprite.name = name;
    sprite.userData["type"] = "star";
    
    const sprite_size = 0.02;
    sprite.scale.set( sprite_size, sprite_size, sprite_size );

    const text_div_el = document.createElement( 'div' );
    text_div_el.addEventListener('click', function(){on_label_click(name)} );
    text_div_el.addEventListener('mouseover', function(ev){on_label_mouseover(ev, name)} );
    text_div_el.addEventListener('mouseout', function(ev){on_label_mouseout(ev, name)} );
    text_div_el.className = 'label';
    text_div_el.textContent = name;
    text_div_el.dataset.type = "star";
    const css2_object = new CSS2DObject( text_div_el );
    
    css2_object.center.set( 0.0, 0.5 );
    sprite.add( css2_object );
    
    sprite.position.set(position.x, position.y, position.z);
    star_group.add( sprite );
    scene.add( star_group );
    name_to_obj[name] = sprite;
}



function onPointerMove( event ) {
    pointer.x = ( event.offsetX / container.clientWidth ) * 2 - 1;
    pointer.y = - ( event.offsetY / container.clientHeight ) * 2 + 1;
}



async function get_and_process_data(scene)
{
    const compressedBuf = await fetch('data', {cache: "no-store"}).then(
        res => res.arrayBuffer()
    );
    const compressed = new Uint8Array(compressedBuf);
    var string = new TextDecoder().decode(fzstd.decompress(compressed));
    all_data = JSON.parse(string);
    for (const [key, value] of Object.entries(all_data["stars"]))
    {
        add_galaxy_view_star(scene, new THREE.Vector3(value.position[0], value.position[1], value.position[2]), key);
    }
}

function main()
{
    scene = new THREE.Scene();
    raycaster = new THREE.Raycaster();
    let initial_center = new THREE.Vector3(10, 10, 0);
    camera.position.set( initial_center.x, initial_center.y+20, initial_center.z+20 );

    scene.add( new THREE.AmbientLight( 0xffffff, 0.2 ) );
    const sun_light = new THREE.PointLight( 0xffffff, 10, 0, 0 );
    sun_light.castShadow = true;
    scene.add( sun_light );

    get_and_process_data(scene);
    
    labelRenderer.setSize( container.clientWidth, container.clientHeight );
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    container.appendChild( labelRenderer.domElement );

    renderer.setSize( container.clientWidth, container.clientHeight );
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    container.appendChild( renderer.domElement ); 

    controls = new OrbitControls( camera, labelRenderer.domElement );
    // controls.enableRotate = false;
    controls.zoomToCursor = false;
    controls.enableDamping = false;
    controls.zoomSpeed = 2.0;
    controls.panSpeed = 1.0;
    controls.target = initial_center;
    controls.update();

    const sectors = 16;
    const rings = 8;
    const divisions = 64;
    gridHelper = new THREE.PolarGridHelper( 20, sectors, rings, divisions );
    gridHelper.position.set(initial_center.x, initial_center.y, initial_center.z);
    scene.add( gridHelper );

    function animate() {
        requestAnimationFrame( animate );
        controls.update();  

        raycaster.setFromCamera( pointer, camera );
        const intersects = raycaster.intersectObjects( star_group.children, false );
        if ( intersects.length > 0 ) {
            if ( intersection_obj != intersects[ 0 ].object ) {
                if ( intersection_obj )
                {
                    unhighlight_obj(intersection_obj, true, intersection_obj.userData.type);
                }
                intersection_obj = intersects[ 0 ].object;
                highlight_obj(intersection_obj, true, intersection_obj.userData.type)
            }
        }
        else {
            if ( intersection_obj )
            {
                unhighlight_obj(intersection_obj, true, intersection_obj.userData.type);
            }
            intersection_obj = null;
        }

        renderer.render( scene, camera );
        labelRenderer.render( scene, camera );
    }
    window.addEventListener( 'resize', onWindowResize, false );
    document.getElementById("home_button").addEventListener("click", click_home);
    onWindowResize();
    animate();
}


function onWindowResize() {
    camera.aspect = container.clientWidth / container.clientHeight;
    camera.updateProjectionMatrix();
    renderer.setSize( container.clientWidth, container.clientHeight );
    labelRenderer.setSize( container.clientWidth, container.clientHeight );
}

labelRenderer.domElement.addEventListener( 'click', on_container_click );
document.addEventListener( 'mousemove', onPointerMove );
window.addEventListener("load", (event) => {
    main();
  });