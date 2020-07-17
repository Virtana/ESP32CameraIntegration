<template>
  <div class="hello">
    <!-- <b-button variant="dark" v-on:click="getFileNames()"> Get Names</b-button> -->
    <div class="mt-3">
      <b-button-group>
        <b-button variant="info" v-on:click="loadPrev()">Prev</b-button>
        <b-button variant="dark" v-on:click="save()">Save Change</b-button>
        <b-button variant="info" v-on:click="loadNext()">Next</b-button>
      </b-button-group>
    </div>
    <div>
      <div>
        <img id="pic" src="../assets/1.jpg" fluid alt="Hard code" height="300px"/>
      </div>
      <label>Enter Name: </label><br>
      <input id="inputName" type="text" v-on:keyup="save()" value="this.eData[(this.loc % this.images.length)].label" v-model="name"> 
    </div>
    <div>
      <vue-csv-downloader
          :data="eData"
          :fields="fields"
      > Download CSV File
      </vue-csv-downloader>
    </div>
  </div>
</template>

<script>

import VueCsvDownloader from 'vue-csv-downloader';
 
export default {
  name: 'HelloWorld',
  props: {
    msg: String
  },
  components: {
      VueCsvDownloader,
  },
  data: function () {
    return {
      images: [],
      loc: 0,
      name: '',
      eData: [],
      fields: ['image', 'label'],
      temp: 'qwertyuiop'
    }
  },
  methods: {
    getFileNames : function (){
      var temp = require.context('@/assets', true, /\.jpg$/)
      this.images = temp.keys()
      for (let z = 0; z < this.images.length; z++) {
        this.images[z] = this.images[z].substr(2)
        temp = {
          image: this.images[z], 
          label: 'unknown'
        }
        this.eData.push(temp)
      }
      console.log(this.eData)
    },
    clear: function (){
      // setTimeout(() => {
      //   document.getElementById("inputName").value = ''
      // }, 1000)
      document.getElementById("inputName").value = ''
      document.getElementById("inputName").placeholder = this.eData[(this.loc % this.images.length)].label
    },
    loadNext: function (){
      // this.save()
      this.loc = this.loc + 1
      this.clear()
      document.getElementById("pic").src = require('../assets/'+this.images[(this.loc % this.images.length)])
    },
    loadPrev: function (){
      //change current variable to next img name in directory
      // this.save()
      this.loc = Math.abs(this.loc - 1)
      this.clear()
      document.getElementById("pic").src = require('../assets/'+this.images[(this.loc % this.images.length)])
    },
    save: function (){
      this.eData[(this.loc % this.images.length)].label = this.name
      console.log(this.eData)
      // this.clear()
    }
  },
  beforeMount(){
    this.getFileNames()
 },
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}
a {
  color: #42b983;
}
</style>
