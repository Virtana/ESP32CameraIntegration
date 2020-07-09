<template>
  <div id=app>
      <!-- <div class="myJ">
        <b-jumbotron bg-variant="info" text-variant="white" header="SPC:ESP 32 Camera Integration" lead="Track 3: Face Identification using ML APIs">
          <p>Milestone 1</p>
          <b-button variant="light"> Export CSV</b-button>
        </b-jumbotron>
      </div> -->
      <HelloWorld msg="Let's get started"/>
  </div>
</template>

<script>
import HelloWorld from './components/HelloWorld.vue'

export default {
  name: 'App',
  components: {
    HelloWorld
  }
}
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin: 2%;
}
.myJ{
  padding: 1px;
  margin: 1px;
}
</style>
