import Vue from 'vue'
import Router from 'vue-router'

import ElementUI from 'element-ui'
import 'element-ui/lib/theme-chalk/index.css'

import App from './App'
import homePage from './pages/home-page'
import articlePage from './pages/article-page'
import searchPage from './pages/search-page'
import userPage from './pages/user-page'
import editPage from './pages/edit-page'

const routes = [
    {
        path: '/',
        name: 'homePage',
        component: homePage
    },
    {
        path: '/article',
        name: 'articlePage',
        component: articlePage,
        props: (route) => ({
            id: route.query.id,
            title: route.params.title,
            img: route.params.img,
            author: route.params.author
        })
    },
    {
        path: '/search',
        name: 'searchPage',
        component: searchPage
    },
    {
        path: '/user',
        name: 'userPage',
        component: userPage
    },
    {
        path: '/edit',
        name: 'editPage',
        component: editPage,
        props: (route) => ({
            id: route.query.id
        })
    },
];

Vue.use(Router)
Vue.use(ElementUI)

Vue.config.productionTip = false

new Vue({
    el: '#app',
    template: '<App/>',
    components: { App },
    router: new Router({
        routes
    })
})
