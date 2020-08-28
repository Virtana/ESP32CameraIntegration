<template>
  <div class="hello">
    <div>
      <b-container class="bv-example-row">
        <b-row class="text-center">
          <b-col>
            <div class="px-3 py-2">
            <b-form-group title="EMPLOYE NAME">
              <b-form-radio-group
                id="btn-radios-3"
                v-model="selected"
                v-on:input="loadNext()"
                :options="options"
                buttons
                stacked
                name="radio-btn-stacked"
              ></b-form-radio-group>
            </b-form-group>
        </div>
          </b-col>
          <b-col cols="8">
            <div>
              <b-card-group>
                <b-card header-tag="header" footer-tag="footer" >
                  <template v-slot:header>
                    <h6 class="mb-0">Who's this?</h6>
                  </template>                  
                    <!-- <h2> {{eData[(this.loc % this.images.length)].label}} </h2> -->
                    <img id="pic" src="" fluid alt="Hard code" height="300px"/>
                  <template v-slot:footer>
                    <b-button v-on:click="loadPrev()" variant="primary">Prev</b-button>
                    <!-- <em>{{selected}}</em> -->
                    <b-button v-on:click="loadNext()" variant="primary">Next</b-button>
                  </template>
                </b-card>
              </b-card-group>
            </div>

            <!-- <div class="mt-3">
              <b-button-group>
                <b-button variant="info" v-on:click="chooseLabel()">Check</b-button>
                <b-button variant="info" v-on:click="loadPrev()">Prev</b-button>
                <b-button variant="info" v-on:click="loadNext()">Next</b-button>
              </b-button-group>
            </div>
            <div>
              <div>
                <img id="pic" src="" fluid alt="Hard code" height="300px"/>
              </div>
              <label>{{selected}} </label><br>
              <input id="inputName" type="text" v-on:keyup="save()" value="this.eData[(this.loc % this.images.length)].label" v-model="name"> 
              <div>
                
              </div>
            </div> -->
            <div>
              <vue-csv-downloader
                  :data="eData"
                  :fields="fields"
              > Download CSV File
              </vue-csv-downloader>
            </div>
          </b-col>
          <b-col>3 of 3</b-col>
        </b-row>
      </b-container>
      <!-- <b-button v-b-toggle.sidebar-1>Toggle Sidebar</b-button> -->
      <!-- <b-sidebar visible id="sidebar-1" title="Choose Employee Name" shadow static>
        <div class="px-3 py-2">
            <b-form-group >
              <b-form-radio-group
                id="btn-radios-3"
                v-model="selected"
                v-on:input="loadNext()"
                :options="options"
                buttons
                stacked
                name="radio-btn-stacked"
              ></b-form-radio-group>
            </b-form-group>
        </div>
      </b-sidebar> -->
    </div>
    <!-- <div class="mt-3">
      <b-button-group>
        <b-button variant="info" v-on:click="chooseLabel()">Check</b-button>
        <b-button variant="info" v-on:click="loadPrev()">Prev</b-button>
        <b-button variant="info" v-on:click="loadNext()">Next</b-button>
      </b-button-group>
    </div>
    <div>
      <div>
        <img id="pic" src="" fluid alt="Hard code" height="300px"/>
      </div>
      <label>{{selected}} </label><br>
      <input id="inputName" type="text" v-on:keyup="save()" value="this.eData[(this.loc % this.images.length)].label" v-model="name"> 
      <div>
        
      </div>
    </div>
    <div>
      <vue-csv-downloader
          :data="eData"
          :fields="fields"
      > Download CSV File
      </vue-csv-downloader>
    </div> -->

  </div>
</template>

<script>

import VueCsvDownloader from 'vue-csv-downloader';
 
export default {
  name: 'ImageLabeller',
  props: {
    msg: String
  },
  components: {
      VueCsvDownloader,
  },
  data: function () {
    return {
      images: [],
      loc: -1,
      name: '',
      eData: [],
      fields: ['label', 'image'],
      labels: ['kohai', 'senpaii'],
      options: [
          { text: 'Captain', value: 'Captain' },
          { text: 'Thor', value: 'Thor' },
          { text: 'Charlie', value: 'Charlie'},
          { text: 'Zendaya', value: 'Zendaya' },
          { text: 'Kim K', value: 'Kim' },
          { text: 'Michael', value: 'Michael' },
          { text: 'Taraji', value: 'Taraji'}
        ],
      selected: ''
    }
  },
  methods: {
    chooseLabel: function (){
      console.log("SELECTEDDDDDD")
      console.log(this.selected)
      let temp = this.selected
      setTimeout(function(){ console.log(temp); }, 3000);
    },
    getFileNames : function (){
      var temp = require.context('@/assets', true, /\.jpg$/)
      this.images = temp.keys()
      for (let z = 0; z < this.images.length; z++) {
        this.images[z] = this.images[z].substr(2)
        temp = {
          label: 'unknown',
          image: this.images[z]
        }
        this.eData.push(temp)
      }
      console.log(this.eData)
    },
    clear: function (){
      this.selected = "unknown"
      // document.getElementById("inputName").value = ''
      // document.getElementById("inputName").placeholder = this.eData[(this.loc % this.images.length)].label
    },
    loadFirst: function (){
      document.getElementById("pic").src = require('../assets/'+this.images[0])
    },
    loadNext: function (){
      this.loc = this.loc + 1
      this.save()
      document.getElementById("pic").src = require('../assets/'+this.images[(this.loc % this.images.length)])
      this.clear()
    },
    loadPrev: function (){
      this.loc = this.loc - 1
      console.log(this.loc)
      // this.clear()
      document.getElementById("pic").src = require('../assets/'+this.images[(this.loc % this.images.length)])
    },
    save: function (){
      // this.eData[(this.loc % this.images.length)].label = this.name
      this.eData[(this.loc % this.images.length)].label = this.selected
      console.log(this.eData)
    }
  },
  beforeMount(){
    this.getFileNames()
  },
  mounted(){
    this.loadFirst()
  }
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
/* SIDE BAR STUUF */
#sidebar-1 {
  position: fixed 
}
</style>
