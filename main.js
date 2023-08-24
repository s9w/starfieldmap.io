import * as THREE from 'three';
import { MapControls } from 'three/addons/controls/MapControls.js';

function add_sphere(scene, position, color)
{
    const geometry = new THREE.SphereGeometry( 1, 32, 16 );
    const material = new THREE.MeshBasicMaterial( { color: color } );
    const cube = new THREE.Mesh( geometry, material );
    cube.position.set(position.x, position.y, position.z);
    scene.add( cube );
}

function main()
{
    const scene = new THREE.Scene();

    const camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
    camera.position.y = 10;

    const renderer = new THREE.WebGLRenderer();
    renderer.setSize( window.innerWidth, window.innerHeight );
    document.body.appendChild( renderer.domElement ); 

    const controls = new MapControls( camera, renderer.domElement );
    controls.enableDamping = true;
    controls.enableRotate = false;

    add_sphere(scene, new THREE.Vector3(0,0,0), 0xffffff);
    add_sphere(scene, new THREE.Vector3(1,0,0), 0xff0000);
    add_sphere(scene, new THREE.Vector3(0,1,0), 0x00ff00);
    add_sphere(scene, new THREE.Vector3(0,0,1), 0x0000ff);

    function animate() {
        requestAnimationFrame( animate );
        controls.update();
        renderer.render( scene, camera );
    }
    animate();
}

main();