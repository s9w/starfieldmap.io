import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';

let scene;
let name_to_obj = {};
let controls;
let star_group = new THREE.Group();
let planets_group = new THREE.Group();
const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
const renderer = new THREE.WebGLRenderer({antialias: true, alpha: true });
let labelRenderer = new CSS2DRenderer();
let container = document.getElementById('glContainer');
let raycaster;
const pointer = new THREE.Vector2(99, 99);
let INTERSECTED;
let system_data;
let star_data;
let mode = "galaxy";

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
}

function get_new_elem(type, content="", classlist="")
{
    let result = document.createElement(type);
    if(content)
        result.textContent = content;
    if(classlist)
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

function set_infobox_star(star_name, star_data)
{
    let infobox_el = document.getElementById("infobox");
    infobox_el.textContent = "";

    let level_indicator = get_new_elem("div", null, "level_indicator");
    level_indicator.appendChild(get_new_elem("div", "LEVEL"));
    level_indicator.appendChild(get_new_elem("div", star_data["level"]));
    infobox_el.appendChild(level_indicator);

    
    infobox_el.appendChild(get_new_elem("div", star_name, "infobox_name"));    

    let attrib_list_el = get_new_elem("div");
    attrib_list_el.id = "infobox_list";
    attrib_list_el.appendChild(get_infobox_attrib_el("SPECTRAL CLASS", star_data["spectral class"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("TEMPERATURE", `${star_data["temperature"]}K`));
    attrib_list_el.appendChild(get_infobox_attrib_el("MASS", `${star_data["mass"]}SM`));
    attrib_list_el.appendChild(get_infobox_attrib_el("RADIUS", star_data["radius"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("MAGNITUDE", star_data["magnitude"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("PLANETS", star_data["planets"]));
    attrib_list_el.appendChild(get_infobox_attrib_el("MOONS", star_data["moons"]));
    infobox_el.appendChild(attrib_list_el);
}

function highlight_obj(obj, with_label, from_type)
{
    if(mode != "galaxy" && from_type == "star")
        return;
    document.getElementById("infobox").classList.add("active");
    set_infobox_star(obj.name, star_data[obj.name]);
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
    document.getElementById("infobox").classList.remove("active");
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
    cam.position.set(0, new_height, 0);
    orbit_controls.target.set(0, orbit_controls.target.y, 0);
    orbit_controls.update();
}

function activate_system(name)
{
    mode = "system";
    document.getElementById("infobox").classList.remove("active");
    document.getElementById("system_indicator").classList.add("active")
    document.getElementById("system_indicator").textContent = name;
    for (const star of star_group.children) {
        star.visible = false;
        star.children[0].visible = false;
    }
    controls.saveState();
    controls.enableZoom = false;
    xy_zero_orbit_controls(controls, 20.0);

    planets_group.clear();
    for (const [key, value] of Object.entries(system_data[name]))
    {
        const geometry = new THREE.SphereGeometry( 2, 16, 12 ); 
        const material = new THREE.MeshStandardMaterial( { color: 0x1f425b } ); 
        const sphere = new THREE.Mesh( geometry, material );
        // sphere.castShadow  = true;
        sphere.receiveShadow = true;
        planets_group.add( sphere );

        sphere.position.set(value["position"][0], value["position"][1], value["position"][2]);
    }
    scene.add( planets_group );
}

function on_label_click(name)
{
    activate_system(name);
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
    if(INTERSECTED !== null)
        activate_system(INTERSECTED.name);
}



function add_sphere(scene, position, name)
{
    const map = new THREE.TextureLoader().load( 'circle.png' );
    const material = new THREE.SpriteMaterial( { map: map, color: 0xff807d  } );
    material.transparent = true;
    material.opacity = 0.5;
    material.sizeAttenuation = false;
    const sprite = new THREE.Sprite( material );
    sprite.name = name;
    sprite.userData["type"] = "star";
    
    const sprite_size = 0.05;
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
    var payload = JSON.parse(string);
    for (const [key, value] of Object.entries(payload.stars))
    {
        add_sphere(scene, new THREE.Vector3(value.position[0], value.position[1], value.position[2]), key);
    }
    system_data = payload.systems;
    star_data = payload.stars;
}

function main()
{
    scene = new THREE.Scene();
    raycaster = new THREE.Raycaster();
    camera.position.y = 40;

    const light = new THREE.AmbientLight( 0xffffff, 0.2 );
    scene.add( light );
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
    renderer.shadowMap.type = THREE.PCFSoftShadowMap; // default THREE.PCFShadowMap
    container.appendChild( renderer.domElement ); 

    controls = new MapControls( camera, labelRenderer.domElement );
    controls.enableRotate = false;
    controls.zoomToCursor = true;
    controls.enableDamping = false;
    controls.zoomSpeed = 2.0;
    controls.panSpeed = 1.0;

    function animate() {
        requestAnimationFrame( animate );
        controls.update();  

        raycaster.setFromCamera( pointer, camera );
        const intersects = raycaster.intersectObjects( star_group.children, false );
        if ( intersects.length > 0 ) {
            if ( INTERSECTED != intersects[ 0 ].object ) {
                if ( INTERSECTED )
                {
                    unhighlight_obj(INTERSECTED, true, INTERSECTED.userData.type);
                }
                INTERSECTED = intersects[ 0 ].object;
                highlight_obj(INTERSECTED, true, INTERSECTED.userData.type)
            }
        }
        else {
            if ( INTERSECTED )
            {
                unhighlight_obj(INTERSECTED, true, INTERSECTED.userData.type);
            }
            INTERSECTED = null;
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

container.addEventListener( 'click', on_container_click );
document.addEventListener( 'mousemove', onPointerMove );
window.addEventListener("load", (event) => {
    main();
  });