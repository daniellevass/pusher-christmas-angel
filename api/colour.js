const Pusher = require('pusher')

const pusher = new Pusher({
  appId: process.env.PUSHER_ANGEL_APP_ID,
  key: process.env.PUSHER_ANGEL_KEY,
  secret: process.env.PUSHER_ANGEL_SECRET,
  cluster:  process.env.PUSHER_ANGEL_CLUSTER,
  useTLS: true
})

module.exports = async (req, res) => {
    let colour = req.body.colour

      try {
        await new Promise((resolve, reject) => {
          pusher.trigger
            ('angel', "new-colour", { colour },
            err => {
              if (err) return reject(err)
              resolve()
            }
          )
        }
        )

        res.status(200).send()
    } catch(e) {
        console.log(e.message)
        res.status(501).send()
    }
}
