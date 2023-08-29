import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';

let scene;
let name_to_obj = {};
let star_group = new THREE.Group();
let planets_group = new THREE.Group();
const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
const renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();
let container = document.getElementById('glContainer');
let raycaster;
const pointer = new THREE.Vector2(99, 99);
let INTERSECTED;
let last_galaxy_camera_pos;
let system_data;

function click_home()
{
    camera.position.set(last_galaxy_camera_pos.x, last_galaxy_camera_pos.y, last_galaxy_camera_pos.z);
    planets_group.clear();
    document.getElementById("system_indicator").classList.remove("active")
    for (const star of star_group.children) {
        star.visible = true;
        star.children[0].visible = true;
    }
}

function highlight_obj(obj, with_label){
    obj.material.opacity = 1.0;
    if(with_label)
    {
        let label_div = obj.children[0].element;
        label_div.classList.add("forcedhover");
    }
}
function unhighlight_obj(obj, with_label){
    obj.material.opacity = 0.5;
    if(with_label)
    {
        let label_div = obj.children[0].element;
        label_div.classList.remove("forcedhover");
    }
}

function activate_system(name)
{
    document.getElementById("system_indicator").classList.add("active")
    document.getElementById("system_indicator").textContent = name;
    for (const star of star_group.children) {
        star.visible = false;
        star.children[0].visible = false;
    }
    last_galaxy_camera_pos = camera.position;
    camera.position.set(0, 50, 0);

    planets_group.clear();
    for (const [key, value] of Object.entries(system_data[name]))
    {
        const geometry = new THREE.SphereGeometry( 3, 16, 12 ); 
        const material = new THREE.MeshBasicMaterial( { color: 0x4ae1aa } ); 
        const sphere = new THREE.Mesh( geometry, material );
        planets_group.add( sphere );

        const wire_geometry = new THREE.WireframeGeometry( geometry ); 
        const wire_material = new THREE.LineBasicMaterial( { color: 0x000000 } );
        const wire_sphere = new THREE.LineSegments( wire_geometry, wire_material );
        planets_group.add( wire_sphere );

        sphere.position.set(value["position"][0], value["position"][1], value["position"][2]);
        wire_sphere.position.set(value["position"][0], value["position"][1], value["position"][2]);
    }
    scene.add( planets_group );
}

function on_label_click(name)
{
    activate_system(name);
}
function on_label_mouseover(name)
{
    highlight_obj(name_to_obj[name], false);
}
function on_label_mouseout(name)
{
    unhighlight_obj(name_to_obj[name], false);
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
    
    const sprite_size = 0.05;
    sprite.scale.set( sprite_size, sprite_size, sprite_size );

    const text_div_el = document.createElement( 'div' );
    text_div_el.addEventListener('click', function(){on_label_click(name)} );
    text_div_el.addEventListener('mouseover', function(){on_label_mouseover(name)} );
    text_div_el.addEventListener('mouseout', function(){on_label_mouseout(name)} );
    text_div_el.className = 'label';
    text_div_el.textContent = name;
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
    const compressedBuf = await fetch('data').then(
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
}

function main()
{
    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111b22 );
    raycaster = new THREE.Raycaster();
    camera.position.y = 40;

    get_and_process_data(scene);
    
    labelRenderer.setSize( container.clientWidth, container.clientHeight );
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    container.appendChild( labelRenderer.domElement );

    renderer.setSize( container.clientWidth, container.clientHeight );
    container.appendChild( renderer.domElement ); 

    const controls = new MapControls( camera, labelRenderer.domElement );
    controls.enableDamping = true;
    controls.enableRotate = false;
    controls.zoomToCursor = true;
    controls.enableDamping = false;
    controls.zoomSpeed = 2.0;

    function animate() {
        requestAnimationFrame( animate );
        controls.update();  

        raycaster.setFromCamera( pointer, camera );
        const intersects = raycaster.intersectObjects( star_group.children, false );
        if ( intersects.length > 0 ) {
            if ( INTERSECTED != intersects[ 0 ].object ) {
                if ( INTERSECTED )
                {
                    unhighlight_obj(INTERSECTED, true);
                }
                INTERSECTED = intersects[ 0 ].object;
                highlight_obj(INTERSECTED, true)
            }
        }
        else {
            if ( INTERSECTED )
            {
                unhighlight_obj(INTERSECTED, true);
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
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    
    renderer.setSize( window.innerWidth, window.innerHeight );
    labelRenderer.setSize( window.innerWidth, window.innerHeight );
}

container.addEventListener( 'click', on_container_click );
document.addEventListener( 'mousemove', onPointerMove );
window.addEventListener("load", (event) => {
    main();
  });