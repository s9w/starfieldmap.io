import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';
import { CSS2DRenderer, CSS2DObject  } from 'three/addons/renderers/CSS2DRenderer.js';

function on_label_click(name)
{
    console.log("clicked, name: " + name);
}

function add_sphere(scene, position, color)
{
    const star_radius = 0.5;
    const geometry = new THREE.SphereGeometry( star_radius, 32, 16 );
    const material = new THREE.MeshBasicMaterial( { color: color } );
    const cube = new THREE.Mesh( geometry, material );

    const moonDiv = document.createElement( 'div' );
    moonDiv.addEventListener('click', function(){on_label_click("test123")} );
    moonDiv.className = 'label';
    moonDiv.textContent = "test 456";
    // moonDiv.style.marginTop = '-0.5em';
    const moonLabel = new CSS2DObject( moonDiv );
    const up_vec = new THREE.Vector3(0, 0, -1);
    
    moonLabel.position.set( up_vec.x*star_radius, up_vec.y*star_radius, up_vec.z*star_radius );
    moonLabel.center.set( 0.5, 0.5 );
    cube.add( moonLabel );
    
    cube.position.set(position.x, position.y, position.z);
    scene.add( cube );
}

const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
const renderer = new THREE.WebGLRenderer();
let labelRenderer = new CSS2DRenderer();
let container = document.getElementById('glContainer');
let raycaster;
const pointer = new THREE.Vector2();
let INTERSECTED;

function onPointerMove( event ) {

    pointer.x = ( event.clientX / window.innerWidth ) * 2 - 1;
    pointer.y = - ( event.clientY / window.innerHeight ) * 2 + 1;

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

    add_sphere(scene, new THREE.Vector3(0,0,0), 0x808080);
    add_sphere(scene, new THREE.Vector3(1,0,0), 0xff0000);
    add_sphere(scene, new THREE.Vector3(0,1,0), 0x00ff00);
    add_sphere(scene, new THREE.Vector3(0,0,1), 0x0000ff);

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