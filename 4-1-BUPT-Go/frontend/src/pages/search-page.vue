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
          <h1>搜索文章</h1>
        </el-col>
        <el-col :span="4">
          <router-link :to="{ name:'searchPage' }">
            <i class="el-icon-search"></i>
          </router-link>
        </el-col>
      </el-row>
    </el-header>
    <div style="text-align: center">
      <el-row class="input-row">
        <el-select style="width:85%" @change="onSearch"
                   v-model="category" placeholder="请选择类别">
          <el-option v-for="item in categoryOptions"
                     :key="item"
                     :label="item"
                     :value="item">
          </el-option>
        </el-select>
      </el-row>
      <el-row class="input-row">
        <el-select style="width:85%" @change="onSearch"
                   v-model="area" placeholder="请选择地区">
          <el-option v-for="item in areaOptions"
                     :key="item"
                     :label="item"
                     :value="item">
          </el-option>
        </el-select>
      </el-row>
    </div>
    <div style="text-align: center; margin: 40px 0" v-if="items.length">
      <article-rich-item-component v-for="item in items" v-bind="item"
                                   :key="item.id" />
    </div>
    <div style="text-align: center; margin: 40px 0" v-else>
      <p>
        没有相关结果 🙄
      </p>
    </div>
  </div>
</template>

<script>
  import axios from 'axios'
  import ajaxPrompt from './helpers/ajax-helper'
  import articleRichItemComponent from './components/article-rich-item-component'
  import options from '../../../common/article-common.json'

  export default {
    name: 'searchPage',
    components: {
      articleRichItemComponent
    },
    data() {
      // Do unshift only once
      if (options.categoryOptions[0])
        options.categoryOptions.unshift('');
      if (options.areaOptions[0])
        options.areaOptions.unshift('');

      return {
        // options
        categoryOptions: options.categoryOptions,
        areaOptions: options.areaOptions,
        // params
        category: '',
        area: '',
        // result
        items: []
      };
    },
    mounted() {
      document.title = '搜索文章 | BUPT Go';
    },
    methods: {
      onSearch() {
        this.items = [];
        if (!this.category && !this.area) {
          // not search but clear all options
          return;
        }

        const url = '/article/search';
        const params = {
          category: this.category,
          area: this.area
        };

        ajaxPrompt(this, axios.get(url, { params }), (res) => {
          // this.items already clear previously
          for (const item of res) {
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
        });
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
