import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';

function on_label_click(name)
{
    console.log("clicked, name: " + name);
}

function add_sphere(scene, position, color, name)
{
    const star_radius = 0.5;
    const geometry = new THREE.SphereGeometry( star_radius, 32, 16 );
    const material = new THREE.MeshBasicMaterial( { color: color } );
    material.transparent = true;
    material.opacity = 0.5;

    const cube = new THREE.Mesh( geometry, material );

    const text_div_el = document.createElement( 'div' );
    text_div_el.addEventListener('click', function(){on_label_click(name)} );
    text_div_el.className = 'label';
    text_div_el.textContent = name;
    const css2_object = new CSS2DObject( text_div_el );
    
    css2_object.position.set( 0, 0, -star_radius );
    css2_object.center.set( 0.5, 0.5 );
    cube.add( css2_object );
    
    cube.position.set(position.x, position.y, position.z);
    scene.add( cube );
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
    camera.position.y = 10;

    
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

    add_sphere(scene, new THREE.Vector3(1,0,0), 0xff807d, "first");
    add_sphere(scene, new THREE.Vector3(0,0,1), 0xff807d, "second");

    function animate() {
        requestAnimationFrame( animate );
        controls.update();  

        raycaster.setFromCamera( pointer, camera );
        const intersects = raycaster.intersectObjects( scene.children, false );
        if ( intersects.length > 0 ) {

            if ( INTERSECTED != intersects[ 0 ].object ) {

                if ( INTERSECTED ) INTERSECTED.material.color.setHex( INTERSECTED.currentHex );

                INTERSECTED = intersects[ 0 ].object;
                INTERSECTED.currentHex = INTERSECTED.material.color.getHex();
                INTERSECTED.material.color.setHex( 0x888800 );

            }

        } else {

            if ( INTERSECTED ) INTERSECTED.material.color.setHex( INTERSECTED.currentHex );

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

document.addEventListener( 'mousemove', onPointerMove );
window.addEventListener("load", (event) => {
    main();
  });