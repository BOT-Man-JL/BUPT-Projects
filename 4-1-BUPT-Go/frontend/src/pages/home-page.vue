<template>
  <div>
    <el-header style="text-align: center">
      <el-row type="flex" align="middle" justify="space-around">
        <el-col :span="4">
          <router-link :to="{ name:'userPage' }">
            <img v-if="userAvatar" :src="userAvatar"
                 style="width: 24px; height: 24px; border-radius: 12px" />
            <i v-else class="el-icon-menu"></i>
          </router-link>
        </el-col>
        <el-col :span="16">
          <h1>最新文章</h1>
        </el-col>
        <el-col :span="4">
          <router-link :to="{ name:'searchPage' }">
            <i class="el-icon-search"></i>
          </router-link>
        </el-col>
      </el-row>
    </el-header>
    <div v-if="items.length">
      <article-rich-item-component v-for="item in items" v-bind="item"
                                   :key="item.id" />
    </div>
    <div v-else style="text-align: center; margin: 40px 0">
      <p>
        目前暂时没有文章 😶
      </p>
    </div>
  </div>
</template>

<script>
  import axios from 'axios'
  import articleRichItemComponent from './components/article-rich-item-component'
  import getCookies from './helpers/cookie-helper'
  export default {
    name: 'homePage',
    components: {
      articleRichItemComponent
    },
    data() {
      var cookies = getCookies();
      return {
        userAvatar: decodeURIComponent(cookies['userAvatar']),
        items: []
      };
    },
    mounted() {
      document.title = '主页 | BUPT Go';
      const url = '/article/recent';

      const loading = this.$loading({ lock: true });
      axios.get(url).then((res) => {
        this.items = [];
        for (const item of res.data) {
          this.items.push({
            id: item._id,
            author: item.author,
            timestamp: new Date(item.timestamp).toLocaleString(),
            title: item.title,
            img: item.img,
            category: item.category,
            area: item.area
          });
        }

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
</style>
