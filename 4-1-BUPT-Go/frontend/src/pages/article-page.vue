<template>
  <div>
    <el-header style="text-align: center">
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="4">
          <router-link :to="{ name:'homePage' }">
            <i class="el-icon-back"></i>
          </router-link>
        </el-col>
        <el-col :span="16">
          <h1>BUPT Go</h1>
        </el-col>
        <el-col :span="4">
          <router-link :to="{ name:'searchPage' }">
            <i class="el-icon-search"></i>
          </router-link>
        </el-col>
      </el-row>
    </el-header>
    <div class="article">
      <img v-bind:src="img" style="width: 100%" />
      <h1>{{ title }}</h1>
      <div>
        <el-row type="flex" align="middle" justify="center"
                style="margin-bottom: 20px">
          <el-col :span="4" align="left">
            <img v-if="authorAvatar" :src="authorAvatar"
                 style="width: 64px; height: 64px; border-radius: 32px" />
          </el-col>
          <el-col :span="12" align="left">
            <p v-if="author">
              {{ author }}
              <br />
              <small v-if="timestamp">
                {{ timestamp }}
              </small>
            </p>
          </el-col>
          <el-col :span="8" align="right">
            <el-tag size="mini" v-if="category">{{ category }}</el-tag>
            <el-tag size="mini" v-if="area">{{ area }}</el-tag>
          </el-col>
        </el-row>
        <el-row type="flex" align="left" justify="left">
          <el-col :span="16" align="left" v-if="contact">
            <span>联系 ☎：{{ contact }}</span>
          </el-col>
          <el-col :span="8" align="left" v-if="cost">
            <span>人均 💰：{{ cost }}</span>
          </el-col>
        </el-row>
        <el-row type="flex" align="left" justify="left" v-if="location">
          <span>地址 🏚：{{ location }}</span>
        </el-row>
      </div>
      <div style="margin: 20px 0" id="content"></div>
    </div>
  </div>
</template>

<script>
  import axios from 'axios'
  import marked from 'marked'
  export default {
    name: 'articlePage',
    props: ['id', 'title', 'img', 'author'],
    data() {
      return {
        authorAvatar: null,
        timestamp: null,
        category: null,
        area: null,
        location: null,
        contact: null,
        cost: null
      };
    },
    mounted() {
      document.title = '文章 | BUPT Go';
      const url = '/article';
      const params = { id: this.id };

      const loading = this.$loading({ lock: true });
      axios.get(url, { params }).then((res) => {
        this.author = res.data.author;
        this.authorAvatar = res.data.authorAvatar;
        this.timestamp = new Date(res.data.timestamp).toLocaleString();
        this.img = res.data.img;
        this.title = res.data.title;
        this.category = res.data.category;
        this.area = res.data.area;
        this.location = res.data.location;
        this.contact = res.data.contact;
        this.cost = res.data.cost;

        document.getElementById('content').innerHTML =
          marked(res.data.text, { sanitize: true });
        document.title = this.title + ' | BUPT Go';

        loading.close();
      }).catch((e) => {
        loading.close();
        this.$message.error({
          message: e.response.data.err, showClose: true
        });
      });
    }
  }
</script>

<style scoped>
  .article {
    margin: 20px;
  }
</style>
