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
          <h1>个人管理</h1>
        </el-col>
        <el-col :span="4">
          <router-link :to="{ name:'editPage' }" v-if="isLogin">
            <i class="el-icon-plus"></i>
          </router-link>
          <router-link :to="{ name:'searchPage' }" v-if="!isLogin">
            <i class="el-icon-search"></i>
          </router-link>
        </el-col>
      </el-row>
    </el-header>
    <!-- Is Login -->
    <div v-if="isLogin">
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="20" align="left">
          <h2>
            <img v-if="userAvatar" :src="userAvatar"
                 style="width: 64px; height: 64px; border-radius: 32px" />
            {{ userName }}
          </h2>
        </el-col>
        <el-col :span="4" align="right">
          <el-button type="primary" size="small" @click="onLogout">
            注销
          </el-button>
        </el-col>
      </el-row>
      <div v-if="items.length">
        <article-thin-item-component v-for="item in items" v-bind="item"
                                     :key="item.id" @remove-item="onRemoveItem" />
      </div>
      <div v-else style="text-align: center; margin: 40px 0">
        <p>
          点击右上角，写下属于你的世界 😘
        </p>
      </div>
    </div>
    <!-- Not Login -->
    <div v-else>
      <el-row class="input-row">
        <el-input style="width:80%"
                  placeholder="请输入账号"
                  v-model="name">
        </el-input>
      </el-row>
      <el-row class="input-row">
        <el-input style="width:80%" type="password"
                  placeholder="请输入密码"
                  v-model="pass">
        </el-input>
      </el-row>
      <!-- Login / Signup -->
      <div v-if="!inSignupPage">
        <el-row class="input-row">
          <el-button type="primary" style="width:80%" @click="onLogin">
            登录
          </el-button>
        </el-row>
        <el-row class="input-row">
          <span>没有账号？</span>
          <el-button @click="inSignupPage = true">注册</el-button>
        </el-row>
      </div>
      <div v-else>
        <el-row class="input-row" style="margin: 10px 20%" justify="center">
          <upload-component text="上传头像" v-model="file" />
        </el-row>
        <el-row class="input-row">
          <el-button type="primary" style="width:80%" @click="onSignup">
            注册
          </el-button>
        </el-row>
        <el-row class="input-row">
          <span>已有账号？</span>
          <el-button @click="inSignupPage = false">登录</el-button>
        </el-row>
      </div>
    </div>
  </div>
</template>

<script>
  import axios from 'axios'
  import ajaxPrompt from './helpers/ajax-helper'
  import getCookies from './helpers/cookie-helper'
  import articleThinItemComponent from './components/article-thin-item-component'
  import uploadComponent from './components/upload-component'

  export default {
    name: 'userPage',
    components: {
      articleThinItemComponent,
      uploadComponent
    },
    data() {
      var cookies = getCookies();
      return {
        // check login
        isLogin: cookies['userName'] != null,
        inSignupPage: false,
        // not login
        name: '',
        pass: '',
        file: null,
        // is login
        userName: cookies['userName'] + '，欢迎发现更大的世界',
        userAvatar: decodeURIComponent(cookies['userAvatar']),
        items: []
      };
    },
    mounted() {
      document.title = '个人管理 | BUPT Go';
      this.checkLogin();
    },
    methods: {
      checkLogin() {
        var cookies = getCookies();
        if (!cookies['userName']) {
          this.isLogin = false;
          this.name = '';
          this.pass = '';
          this.file = null;
          return;
        }

        this.isLogin = true;
        this.userName = cookies['userName'] + '，欢迎发现更大的世界';
        this.userAvatar = decodeURIComponent(cookies['userAvatar']);

        const url = '/article/user';

        ajaxPrompt(this, axios.get(url), (res) => {
          this.items = [];
          for (const item of res) {
            this.items.push({
              id: item._id,
              timestamp: new Date(item.timestamp).toLocaleString(),
              title: item.title
            });
          }
        });
      },
      postAction(url, data) {
        ajaxPrompt(this, axios.post(url, data), (res) => {
          this.$message({
            message: res.msg, showClose: true
          });
          this.checkLogin();
        });
      },
      onLogin() {
        const url = '/user/login';
        const data = { name: this.name, pass: this.pass };
        this.postAction(url, data);
      },
      onSignup() {
        const url = '/user/signup';
        const data = new FormData();
        data.append('name', this.name);
        data.append('pass', this.pass);
        data.append('avatar', this.file);
        this.postAction(url, data);
      },
      onLogout() {
        const url = '/user/logout';
        this.postAction(url, null);
      },
      onRemoveItem() {
        this.checkLogin();
      }
    }
  }
</script>

<style scoped>
  .input-row {
    margin: 10px 0;
    text-align: center;
  }
</style>
