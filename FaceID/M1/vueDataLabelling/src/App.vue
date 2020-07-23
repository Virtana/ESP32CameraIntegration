<template>
  <div id=app>
      <!-- <div class="myJ">
        <b-jumbotron bg-variant="info" text-variant="white" header="SPC:ESP 32 Camera Integration" lead="Track 3: Face Identification using ML APIs">
        </b-jumbotron>
      </div> -->
      <ImageLabeller msg="Let's get started"/>
  </div>
</template>

<script>
import ImageLabeller from './components/ImageLabeller.vue'

export default {
  name: 'App',
  components: {
    ImageLabeller
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
