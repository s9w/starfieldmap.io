import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';

let name_to_obj = {};

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

function on_label_click(name)
{
    console.log("clicked, name: " + name);
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
        console.log("container clicked while on something");
}



function add_sphere(scene, position, color, name)
{
    const star_radius = 0.5;

    const map = new THREE.TextureLoader().load( 'circle.png' );
    const material = new THREE.SpriteMaterial( { map: map, color: 0xff807d  } );
    material.transparent = true;
    material.opacity = 1;

    const sprite = new THREE.Sprite( material );

    const text_div_el = document.createElement( 'div' );
    text_div_el.addEventListener('click', function(){on_label_click(name)} );
    text_div_el.addEventListener('mouseover', function(){on_label_mouseover(name)} );
    text_div_el.addEventListener('mouseout', function(){on_label_mouseout(name)} );
    text_div_el.className = 'label';
    text_div_el.textContent = name;
    const css2_object = new CSS2DObject( text_div_el );
    
    css2_object.position.set( 1.5 * star_radius, 0, 0 );
    css2_object.center.set( 0.0, 0.5 );
    sprite.add( css2_object );
    
    sprite.position.set(position.x, position.y, position.z);
    scene.add( sprite );
    name_to_obj[name] = sprite;
}

const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
const renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();
let container = document.getElementById('glContainer');
let raycaster;
const pointer = new THREE.Vector2();
let INTERSECTED;

function onPointerMove( event ) {
    pointer.x = ( event.offsetX / container.clientWidth ) * 2 - 1;
    pointer.y = - ( event.offsetY / container.clientHeight ) * 2 + 1;
}

function main()
{
    const scene = new THREE.Scene();
    raycaster = new THREE.Raycaster();
    camera.position.y = 20;
    
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

    add_sphere(scene, new THREE.Vector3(1,0,0), 0xff807d, "ALPHA CENTAURI");
    add_sphere(scene, new THREE.Vector3(0,1,1), 0xff807d, "PORRIMA");

    function animate() {
        requestAnimationFrame( animate );
        controls.update();  

        raycaster.setFromCamera( pointer, camera );
        const intersects = raycaster.intersectObjects( scene.children, false );
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